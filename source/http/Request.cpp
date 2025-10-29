#include "../../include/http/Request.hpp"
#include "../../include/colors.hpp"

#include <sstream>
#include <algorithm>
#include <iostream>
#include <cstdlib>   // std::strtoul

// --------- small helpers ---------
static std::string trim(const std::string& s) {
	size_t start = s.find_first_not_of(" \t\r\n");
	size_t end   = s.find_last_not_of(" \t\r\n");
	if (start == std::string::npos || end == std::string::npos) return "";
	return s.substr(start, end - start + 1);
}

std::string Request::toLower(const std::string& s) {
	std::string r = s;
	for (size_t i = 0; i < r.size(); ++i) {
		char c = r[i];
		if (c >= 'A' && c <= 'Z') r[i] = static_cast<char>(c - 'A' + 'a');
	}
	return r;
}

// --------- chunked decoding (simple, assumes well-formed) ---------
std::string Request::dechunkBody(const std::string& chunked) {
	std::string result;
	size_t pos = 0;

	while (pos < chunked.size()) {
		size_t line_end = chunked.find("\r\n", pos);
		if (line_end == std::string::npos) break;

		std::string size_str = chunked.substr(pos, line_end - pos);
		size_t chunk_size = 0;
		std::istringstream iss(size_str);
		iss >> std::hex >> chunk_size;

		pos = line_end + 2; // skip CRLF

		if (chunk_size == 0) {
			// optional trailers are ignored; consume final CRLF if present
			// (safe to stop here for our purposes)
			break;
		}

		if (pos + chunk_size > chunked.size()) {
			// incomplete data; return what we have (caller can append more later)
			break;
		}

		result.append(chunked, pos, chunk_size);
		pos += chunk_size;

		// expect CRLF after chunk data
		if (pos + 1 < chunked.size() && chunked[pos] == '\r' && chunked[pos + 1] == '\n')
			pos += 2;
		else
			break;
	}

	return result;
}

// --------- parser ---------
bool Request::parse(const std::string& raw) {
	// start-line
	size_t pos = raw.find("\r\n");
	if (pos == std::string::npos) return false;

	std::string start_line = raw.substr(0, pos);
	std::istringstream sl(start_line);
	if (!(sl >> method >> target >> version)) return false;

	// headers
	size_t header_end = raw.find("\r\n\r\n");
	if (header_end == std::string::npos) return false;

	std::string headers_block = raw.substr(pos + 2, header_end - (pos + 2));
	std::istringstream hl(headers_block);
	std::string line;

	headers.clear();
	while (std::getline(hl, line)) {
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		size_t colon = line.find(':');
		if (colon != std::string::npos) {
			std::string key   = trim(line.substr(0, colon));
			std::string value = trim(line.substr(colon + 1));
			if (!key.empty())
				headers[toLower(key)] = value; // store with lowercase keys
		}
	}

	if (header_end + 4 < raw.size())
		body = raw.substr(header_end + 4);
	else
		body.clear();

	if (isChunked()) {
		body = dechunkBody(body);
		std::ostringstream oss;
		oss << body.size();
		headers["content-length"]     = oss.str();
		// keep transfer-encoding as-is or erase it; we keep it to be benign
		// headers.erase("transfer-encoding");
	}

	return true;
}

std::string Request::getTargetPath() const {
    size_t pos = target.find('?');
    if (pos == std::string::npos)
        return target;
    return target.substr(0, pos);
}

std::string Request::getQueryString() const {
    size_t pos = target.find('?');
    if (pos == std::string::npos || pos + 1 >= target.size())
        return "";
    return target.substr(pos + 1);
}

std::string Request::getPathInfo() const {
    return "";
}


// --------- accessors / helpers ---------
std::string Request::getHeader(const std::string& key) const {
	std::map<std::string, std::string>::const_iterator it = headers.find(toLower(key));
	if (it != headers.end())
		return it->second;
	return "";
}

bool Request::hasHeader(const std::string& key) const {
	return headers.find(toLower(key)) != headers.end();
}

bool Request::headerEquals(const std::string& key, const std::string& val) const {
	std::map<std::string, std::string>::const_iterator it = headers.find(toLower(key));
	if (it == headers.end()) return false;
	std::string a = trim(it->second);
	std::string b = trim(val);
	return toLower(a) == toLower(b);
}

size_t Request::contentLength() const {
	std::map<std::string, std::string>::const_iterator it = headers.find("content-length");
	if (it == headers.end()) return 0;
	const std::string &v = it->second;
	char *endp = 0;
	unsigned long n = std::strtoul(v.c_str(), &endp, 10);
	if (!endp || *endp != '\0') return 0; // invalid → treat as 0
	return static_cast<size_t>(n);
}

bool Request::isChunked() const {
	std::map<std::string, std::string>::const_iterator it = headers.find("transfer-encoding");
	if (it == headers.end()) return false;
	std::string v = toLower(it->second);
	return v.find("chunked") != std::string::npos;
}

bool Request::hasExpect100() const {
	std::map<std::string, std::string>::const_iterator it = headers.find("expect");
	if (it == headers.end()) return false;
	return toLower(trim(it->second)) == "100-continue";
}

bool Request::isValidMethod() const {
	if (method == "GET")    return true;
	if (method == "POST")   return true;
	if (method == "DELETE") return true;
	if (method == "HEAD")   return true;
	if (method == "PUT")    return true;
	return false;
}

void Request::debugPrint() const {
	std::cout << BOLD << CYAN << "===== HTTP Request =====" << RESET << std::endl;

	std::cout << GREEN << "Method:  " << RESET << method << std::endl;
	std::cout << BLUE  << "Target:  " << RESET << target << std::endl;
	std::cout << CYAN  << "Version: " << RESET << version << std::endl;

	std::cout << MAGENTA << "--- Headers ---" << RESET << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
		std::cout << MAGENTA << it->first << ": " << RESET
		          << YELLOW  << it->second << RESET << std::endl;
	}

	if (!body.empty()) {
		std::cout << GREEN << "--- Body (" << body.size() << " bytes"
		          << ", showing up to 512) ---" << RESET << std::endl;
		const size_t cap = 512;
		std::string preview = (body.size() > cap) ? (body.substr(0, cap) + "...") : body;
		std::cout << WHITE << preview << RESET << std::endl;
	}

	std::cout << BOLD << CYAN << "========================" << RESET << std::endl;
}
