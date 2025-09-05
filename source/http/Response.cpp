#include "../../include/http/Response.hpp"
#include <fstream>

const std::map<int, std::string> Response::reasonPhrases = Response::initReasonPhrases();

std::map<int, std::string> Response::initReasonPhrases() {
	std::map<int, std::string> m;
	m[200] = "OK";
	m[201] = "Created";
	m[204] = "No Content";
	m[301] = "Moved Permanently";
	m[302] = "Found";
	m[400] = "Bad Request";
	m[401] = "Unauthorized";
	m[403] = "Forbidden";
	m[404] = "Not Found";
	m[405] = "Method Not Allowed";
	m[413] = "Payload Too Large";
	m[500] = "Internal Server Error";
	m[501] = "Not Implemented";
	m[502] = "Bad Gateway";
	m[503] = "Service Unavailable";
	return m;
}

Response::Response() : statusCode(200), reasonPhrase("OK"), contentType("text/html"), body("") {}

void Response::setStatus(int code) {
	statusCode = code;
	std::map<int, std::string>::const_iterator it = reasonPhrases.find(code);
	if (it != reasonPhrases.end())
		reasonPhrase = it->second;
	else
		reasonPhrase = "Unknown";
}

void Response::setContentType(const std::string& ctype) {
	contentType = ctype;
}

void Response::setBody(const std::string& b) {
	body = b;
}

void Response::setErrorBody(int code, const std::string& errorDir) {
	setStatus(code);
	std::ostringstream filename;
	filename << errorDir << "/" << code << ".html";

	std::ifstream file(filename.str().c_str());
	if (file) {
		std::ostringstream buffer;
		buffer << file.rdbuf();
		setBody(buffer.str());
		setContentType("text/html");
	} else {
		std::ostringstream fallback;
		fallback << "<h1>" << code << " " << reasonPhrase << "</h1>";
		setBody(fallback.str());
		setContentType("text/html");
	}
}

std::string Response::build() const {
	std::ostringstream ss;
	ss << "HTTP/1.1 " << statusCode << " " << reasonPhrase << "\r\n";
	ss << "Content-Type: " << contentType << "\r\n";
	ss << "Content-Length: " << body.size() << "\r\n";
	ss << "Connection: close\r\n";
	ss << "\r\n";
	ss << body;
	return ss.str();
}
