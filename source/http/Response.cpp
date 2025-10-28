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

Response::Response() : statusCode(200), reasonPhrase("OK"), contentType("text/html"), body(""), redirectLocation(""), is_cgi(false) {}

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

void Response::setRedirect(const std::string& location) {
	redirectLocation = location;
	if (body.empty()) {
		std::ostringstream ss;
		ss << "<html><head><meta http-equiv=\"refresh\" content=\"0;url=" << location << "\"></head>";
		ss << "<body><h1>Redirecting...</h1><p>If you are not redirected automatically, ";
		ss << "click <a href=\"" << location << "\">here</a>.</p></body></html>";
		body = ss.str();
	}
}

void Response::setErrorBody(int code, const std::string& errorDir) {
	setStatus(code);
	
	if (!errorDir.empty()) {
		std::ostringstream filename;
		filename << errorDir << "/" << code << ".html";
		std::ifstream file(filename.str().c_str());
		if (file) {
			std::ostringstream buffer;
			buffer << file.rdbuf();
			setBody(buffer.str());
			setContentType("text/html");
			return;
		}
	}

	std::ostringstream html;
	html << "<!DOCTYPE html>\n"
		<< "<html lang=\"en\">\n"
		<< "<head>\n"
		<< "    <meta charset=\"UTF-8\">\n"
		<< "    <title>webserv</title>\n"
		<< "    <link href=\"https://fonts.googleapis.com/css2?family=Comic+Neue&display=swap\" rel=\"stylesheet\">\n"
		<< "    <style>\n"
		<< "        * { margin: 0; padding: 0; font-family: 'Comic Neue', cursive, sans-serif; }\n"
		<< "        html, body { width: 100%; min-height: 100%; }\n"
		<< "        body {\n"
		<< "            display: flex;\n"
		<< "            justify-content: center;\n"
		<< "            align-items: center;\n"
		<< "            height: 100vh;\n"
		<< "            background-image: url('https://img.freepik.com/free-vector/red-caution-barrier-tape-with-striped-pattern_107791-33624.jpg');\n"
		<< "            background-size: cover;\n"
		<< "        }\n"
		<< "        .error h1 {\n"
		<< "            z-index: 1000;\n"
		<< "            font-size: 48px;\n"
		<< "            color: #fff;\n"
		<< "            text-shadow: 2px 2px 4px rgba(0,0,0,0.8);\n"
		<< "        }\n"
		<< "    </style>\n"
		<< "</head>\n"
		<< "<body>\n"
		<< "    <div class=\"error\">\n"
		<< "        <h1>Error " << code << ": " << reasonPhrase << "</h1>\n"
		<< "    </div>\n"
		<< "</body>\n"
		<< "</html>";
	
	setBody(html.str());
	setContentType("text/html");
}

std::string Response::build() const {
	std::ostringstream ss;
	ss << "HTTP/1.1 " << statusCode << " " << reasonPhrase << "\r\n";
	
	if (!redirectLocation.empty()) {
		ss << "Location: " << redirectLocation << "\r\n";
	}
	
	ss << "Content-Type: " << contentType << "\r\n";
	ss << "Content-Length: " << body.size() << "\r\n";
	ss << "Connection: close\r\n";
	ss << "\r\n";
	ss << body;
	return ss.str();
}
