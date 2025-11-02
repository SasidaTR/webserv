/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jvittoz <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/02 19:49:00 by jvittoz           #+#    #+#             */
/*   Updated: 2025/11/02 19:49:04 by jvittoz          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/cgi/CGIEnvironment.hpp"
#include "../../include/http/Request.hpp"
#include <cctype>
#include <cstdlib>

static std::string toLower(std::string s) {
	for (size_t i = 0; i < s.size(); ++i) s[i] = static_cast<unsigned char>(std::tolower(s[i]));
	return s;
}

static std::string httpHeaderNameToEnvKey(const std::string& name) {
	std::string res = "HTTP_";
	for (size_t i = 0; i < name.size(); ++i) {
		char c = name[i];
		if (c == '-') res += '_';
		else res += static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
	}
	return res;
}

static bool isAllDigits(const std::string& s) {
	for (size_t i = 0; i < s.size(); ++i)
		if (!std::isdigit(static_cast<unsigned char>(s[i]))) return false;
	return !s.empty();
}

void CGIEnvironment::addBasicVariables(std::map<std::string, std::string>& env,
	const Request& req, const std::string& scriptPath) {
	env["REQUEST_METHOD"] = req.getMethod();
	env["SERVER_SOFTWARE"] = "webserv/1.0";
	env["SERVER_PROTOCOL"] = "HTTP/1.1";
	env["GATEWAY_INTERFACE"] = "CGI/1.1";
	env["SCRIPT_FILENAME"] = scriptPath;
	env["PATH_INFO"] = req.getTarget(); 
}

void CGIEnvironment::addUrlVariables(std::map<std::string, std::string>& env, const Request& req) {
	const std::string& url = req.getTarget();
	size_t q = url.find('?');
	env["SCRIPT_NAME"] = (q == std::string::npos ? url : url.substr(0, q));
	env["QUERY_STRING"] = (q == std::string::npos ? "" : url.substr(q + 1));
	env["REQUEST_URI"] = url;
}

void CGIEnvironment::addHttpHeaders(std::map<std::string, std::string>& env, const Request& req) {
	const std::map<std::string, std::string>& headers = req.getHeaders();
	bool haveServerName = false, haveServerPort = false;
	std::string transferEncoding, contentLength;

	for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
		const std::string nameLower = toLower(it->first);
		if (nameLower == "host") {
			size_t colon = it->second.find(':');
			env["SERVER_NAME"] = (colon == std::string::npos ? it->second : it->second.substr(0, colon));
			env["SERVER_PORT"] = (colon == std::string::npos ? "8080" : it->second.substr(colon + 1));
			haveServerName = haveServerPort = true;
		} else if (nameLower == "content-type") env["CONTENT_TYPE"] = it->second;
		else if (nameLower == "content-length") contentLength = it->second;
		else if (nameLower == "transfer-encoding") transferEncoding = toLower(it->second);
		env[httpHeaderNameToEnvKey(it->first)] = it->second;
	}

	if (transferEncoding.find("chunked") == std::string::npos && isAllDigits(contentLength))
		env["CONTENT_LENGTH"] = contentLength;

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
	std::vector<std::string> envStrings;
	envStrings.reserve(environment.size());
	std::vector<char*> envArray;
	envArray.reserve(environment.size() + 1);

	for (std::map<std::string, std::string>::const_iterator it = environment.begin(); it != environment.end(); ++it)
		envStrings.push_back(it->first + "=" + it->second);

	for (size_t i = 0; i < envStrings.size(); ++i)
		envArray.push_back(const_cast<char*>(envStrings[i].c_str()));

	envArray.push_back(NULL);
	return envArray;
}
