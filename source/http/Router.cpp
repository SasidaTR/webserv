#include "../../include/http/Router.hpp"

Router::Router(const ServerFlat& s) : server(s) {}

std::string Router::getContentType(const std::string& path) const {
	if (path.size() >= 5 && path.substr(path.size() - 5) == ".html") return "text/html";
	if (path.size() >= 4 && path.substr(path.size() - 4) == ".css") return "text/css";
	if (path.size() >= 3 && path.substr(path.size() - 3) == ".js") return "application/javascript";
	if (path.size() >= 4 && path.substr(path.size() - 4) == ".png") return "image/png";
	if (path.size() >= 4 && path.substr(path.size() - 4) == ".jpg") return "image/jpeg";
	return "text/plain";
}

bool Router::readFile(const std::string& path, std::string& content) const {
	std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
	if (!file) return false;
	std::ostringstream ss;
	ss << file.rdbuf();
	content = ss.str();
	return true;
}

Response Router::route(const Request& req) const {
	Response resp;

	if (req.getMethod() != "GET") {
		resp.setStatus("HTTP/1.1 405 Method Not Allowed");
		resp.setContentType("text/html");
		resp.setBody("<h1>405 Method Not Allowed</h1>");
		return resp;
	}

	std::string path = server.root + "/" + server.index;
	if (req.getTarget() == "/style.css")
		path = "./html/style.css";

	std::string body;
	if (readFile(path, body)) {
		resp.setStatus("HTTP/1.1 200 OK");
		resp.setContentType(getContentType(path));
		resp.setBody(body);
	} else {
		resp.setStatus("HTTP/1.1 404 Not Found");
		resp.setContentType("text/html");
		resp.setBody("<h1>404 Not Found</h1>");
	}

	return resp;
}
