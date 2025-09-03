#include "../../include/http/Request.hpp"
#include "../../include/colors.hpp"
#include <sstream>
#include <algorithm>
#include <iostream>

static std::string trim(const std::string& s) {
	size_t start = s.find_first_not_of(" \t\r\n");
	size_t end = s.find_last_not_of(" \t\r\n");
	if (start == std::string::npos || end == std::string::npos) return "";
	return s.substr(start, end - start + 1);
}

bool Request::parse(const std::string& raw) {
	size_t pos = raw.find("\r\n");
	if (pos == std::string::npos) return false;

	std::string start_line = raw.substr(0, pos);
	std::istringstream sl(start_line);
	if (!(sl >> method >> target >> version)) return false;

	size_t header_end = raw.find("\r\n\r\n");
	if (header_end == std::string::npos) return false;

	std::string headers_block = raw.substr(pos + 2, header_end - (pos + 2));
	std::istringstream hl(headers_block);
	std::string line;
	while (std::getline(hl, line)) {
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		size_t colon = line.find(':');
		if (colon != std::string::npos) {
			std::string key = trim(line.substr(0, colon));
			std::string value = trim(line.substr(colon + 1));
			if (!key.empty())
				headers[key] = value;
		}
	}

	if (header_end + 4 < raw.size())
		body = raw.substr(header_end + 4);
	else
		body.clear();

	return true;
}

std::string Request::getHeader(const std::string& key) const {
	std::map<std::string, std::string>::const_iterator it = headers.find(key);
	if (it != headers.end())
		return it->second;
	return "";
}

void Request::debugPrint() const {
	std::cout << BOLD << CYAN << "===== HTTP Request =====" << RESET << std::endl;

	std::cout << GREEN << "Method:  " << RESET << method << std::endl;
	std::cout << BLUE  << "Target:  " << RESET << target << std::endl;
	std::cout << CYAN  << "Version: " << RESET << version << std::endl;

	std::cout << MAGENTA << "--- Headers ---" << RESET << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
		std::cout << MAGENTA << it->first << ": " << RESET 
		          << YELLOW << it->second << RESET << std::endl;
	}

	if (!body.empty()) {
		std::cout << GREEN << "--- Body ---" << RESET << std::endl;
		std::cout << WHITE << body << RESET << std::endl;
	}

	std::cout << BOLD << CYAN << "========================" << RESET << std::endl;
}
