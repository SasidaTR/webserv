#include "../include/http/Request.hpp"
#include "../include/http/Response.hpp"
#include "../include/configuration/configParse.hpp"

#include <fstream>
#include <sstream>
#include <unistd.h>
#include <sys/socket.h>
#include <cerrno>

static bool send_all(int fd, const std::string& data) {
	size_t sent = 0;
	while (sent < data.size()) {
		ssize_t n = send(fd, data.data() + sent, data.size() - sent, 0);
		if (n > 0) {
			sent += static_cast<size_t>(n);
		} else if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
			usleep(1000);
			continue;
		} else {
			return false;
		}
	}
	return true;
}

static bool recv_request_once(int fd, std::string& req) {
	char buf[8192];
	ssize_t n = recv(fd, buf, sizeof(buf), 0);
	if (n <= 0) return false;
	req.assign(buf, buf + n);
	return true;
}

void handle_client(int client_fd, const ServerFlat& s) {
	std::string req;
	if (!recv_request_once(client_fd, req)) {
		close(client_fd);
		return;
	}
	Request r;
	Response resp;

	if (!r.parse_start_line(req)) {
		resp.setStatus("HTTP/1.1 400 Bad Request");
		resp.setContentType("text/html");
		resp.setBody("<h1>Bad Request</h1>");
		send_all(client_fd, resp.build());
		close(client_fd);
		return;
	}

	std::string body;
	std::string ctype = "text/html";
	std::string filepath = s.index;

	if (r.getMethod() == "GET" && r.getTarget() == "/style.css") {
		std::ifstream file("./html/style.css", std::ios::in | std::ios::binary);
		if (file) {
			std::ostringstream ss;
			ss << file.rdbuf();
			body = ss.str();
			ctype = "text/css";
		} else {
			body = "<h1>html file not found</h1>";
		}
	} else {
		std::ifstream file(filepath.c_str(), std::ios::in | std::ios::binary);
		if (file) {
			std::ostringstream ss;
			ss << file.rdbuf();
			body = ss.str();
		} else {
			body = "<h1>html file not found</h1>";
		}
	}

	resp.setStatus("HTTP/1.1 200 OK");
	resp.setContentType(ctype);
	resp.setBody(body);

	send_all(client_fd, resp.build());
	close(client_fd);
}
