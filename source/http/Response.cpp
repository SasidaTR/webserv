#include "../../include/http/Response.hpp"

Response::Response() : status("HTTP/1.1 200 OK"), contentType("text/html"), body("") {}

void Response::setStatus(const std::string& s) {
	status = s;
}

void Response::setContentType(const std::string& ctype) {
	contentType = ctype;
}

void Response::setBody(const std::string& b) {
	body = b;
}

std::string Response::build() const {
	std::ostringstream ss;
	ss << status << "\r\n";
	ss << "Content-Type: " << contentType << "\r\n";
	ss << "Content-Length: " << body.size() << "\r\n";
	ss << "Connection: close\r\n";
	ss << "\r\n";
	ss << body;
	return ss.str();
}
