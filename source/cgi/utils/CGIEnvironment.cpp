#include "../../../include/cgi/utils/CGIEnvironment.hpp"
#include "../../../include/http/Request.hpp"
#include <cctype>
#include <cstdlib>

static std::string toLower(std::string s) {
	for (size_t i = 0; i < s.size(); ++i) s[i] = static_cast<unsigned char>(std::tolower(s[i]));
	return s;
}

static std::string httpHeaderNameToEnvKey(std::string name) {
	for (size_t i = 0; i < name.size(); ++i) {
		char &c = name[i];
		if (c == '-') c = '_';
		else c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
	}
	return "HTTP_" + name;
}

static bool isAllDigits(const std::string& s) {
	if (s.empty()) return false;
	for (size_t i = 0; i < s.size(); ++i)
		if (!std::isdigit(static_cast<unsigned char>(s[i]))) return false;
	return true;
}

void CGIEnvironment::addBasicVariables(std::map<std::string, std::string>& env,
	const Request& req, const std::string& scriptPath) {
	env["REQUEST_METHOD"] = req.getMethod();
	env["SERVER_SOFTWARE"] = "webserv/1.0";
	env["SERVER_PROTOCOL"] = "HTTP/1.1";
	env["GATEWAY_INTERFACE"] = "CGI/1.1";
	env["SCRIPT_FILENAME"] = scriptPath;
	env["PATH_INFO"] = "";
}

void CGIEnvironment::addUrlVariables(std::map<std::string, std::string>& env, const Request& req) {
	const std::string url = req.getTarget();
	size_t q = url.find('?');
	if (q == std::string::npos) {
		env["SCRIPT_NAME"] = url;
		env["QUERY_STRING"] = "";
		env["REQUEST_URI"] = url;
	} else {
		env["SCRIPT_NAME"] = url.substr(0, q);
		env["QUERY_STRING"] = url.substr(q + 1);
		env["REQUEST_URI"] = url;
	}
}

void CGIEnvironment::addHttpHeaders(std::map<std::string, std::string>& env, const Request& req) {
	const std::map<std::string, std::string>& headers = req.getHeaders();
	bool haveServerName = false, haveServerPort = false;
	std::string transferEncoding, contentLength;

	for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
		const std::string nameLower = toLower(it->first);
		if (nameLower == "host") {
			const std::string& v = it->second;
			size_t colon = v.find(':');
			if (colon != std::string::npos) {
				env["SERVER_NAME"] = v.substr(0, colon);
				env["SERVER_PORT"] = v.substr(colon + 1);
				haveServerName = haveServerPort = true;
			} else {
				env["SERVER_NAME"] = v;
				haveServerName = true;
			}
		} else if (nameLower == "content-type") {
			env["CONTENT_TYPE"] = it->second;
		} else if (nameLower == "content-length") {
			contentLength = it->second;
		} else if (nameLower == "transfer-encoding") {
			transferEncoding = toLower(it->second);
		}
		env[httpHeaderNameToEnvKey(it->first)] = it->second;
	}

	if (transferEncoding.find("chunked") == std::string::npos && isAllDigits(contentLength)) {
		env["CONTENT_LENGTH"] = contentLength;
	}

	if (haveServerName && !haveServerPort) env["SERVER_PORT"] = "8080";
}

void CGIEnvironment::addServerDefaults(std::map<std::string, std::string>& env) {
	if (env.find("SERVER_NAME") == env.end()) env["SERVER_NAME"] = "localhost";
	if (env.find("SERVER_PORT") == env.end()) env["SERVER_PORT"] = "8080";
}

std::map<std::string, std::string> CGIEnvironment::prepare(const Request& req, const std::string& scriptPath) {
	std::map<std::string, std::string> env;
	addBasicVariables(env, req, scriptPath);
	addUrlVariables(env, req);
	addHttpHeaders(env, req);
	addServerDefaults(env);
	return env;
}

std::vector<char*> CGIEnvironment::createEnvArray(const std::map<std::string, std::string>& environment) {
	static std::vector<std::string> envStrings;
	envStrings.clear();

	std::vector<char*> envArray;
	envArray.reserve(environment.size() + 1);

	for (std::map<std::string, std::string>::const_iterator it = environment.begin(); it != environment.end(); ++it)
		envStrings.push_back(it->first + "=" + it->second);

	for (size_t i = 0; i < envStrings.size(); ++i)
		envArray.push_back(const_cast<char*>(envStrings[i].c_str()));

	envArray.push_back(NULL);
	return envArray;
}
