#include "../include/webserv.hpp"

std::string build_response(const std::string& body, const std::string& ctype) {
	std::ostringstream len;
	len << body.size();
	return "HTTP/1.1 200 OK\r\n"
		"Content-Type: " + ctype + "\r\n"
		"Content-Length: " + len.str() + "\r\n"
		"\r\n" +
		body;
}

void handle_client(int client_fd) {
	char buf[1024];
	ssize_t n = recv(client_fd, buf, sizeof(buf), 0);
	if (n <= 0) {
		close(client_fd);
		return;
	}
	std::string req(buf, n);
	std::string body;
	std::string ctype = "text/html";

	if (req.find("GET /style.css") == 0) {
		std::ifstream file("./html/style.css");
		if (file) {
			std::ostringstream ss;
			ss << file.rdbuf();
			body = ss.str();
			ctype = "text/css";
		}
	} else {
		std::ifstream file("./html/index.html");
		if (file) {
			std::ostringstream ss;
			ss << file.rdbuf();
			body = ss.str();
		} else {
			body = "<h1>html file not found</h1>";
		}
	}

	std::string response = build_response(body, ctype);
	send(client_fd, response.c_str(), response.size(), 0);
	close(client_fd);
}
