#include "../include/hpp/webserv.hpp"
#include "../include/http/Request.hpp"
#include <fstream>

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

static std::string build_response(const std::string& body, const std::string& ctype) {
	std::ostringstream ss;
	ss << "HTTP/1.1 200 OK\r\n";
	ss << "Content-Type: " << ctype << "\r\n";
	ss << "Content-Length: " << body.size() << "\r\n";
	ss << "Connection: close\r\n";
	ss << "\r\n";
	ss << body;
	return ss.str();
}

static bool recv_request_once(int fd, std::string& req) {
	char buf[8192];
	ssize_t n = recv(fd, buf, sizeof(buf), 0);
	if (n <= 0) return false;
	req.assign(buf, buf + n);
	return true;
}

void handle_client(int client_fd) {
	std::string req;
	if (!recv_request_once(client_fd, req)) {
		close(client_fd);
		return;
	}
	Request r;
	if (!r.parse_start_line(req)) {
		const std::string b = "<h1>Bad Request</h1>";
		const std::string rsp = build_response(b, "text/html");
		send_all(client_fd, rsp);
		close(client_fd);
		return;
	}
	std::string body;
	std::string ctype = "text/html";
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
		std::ifstream file("./html/index.html", std::ios::in | std::ios::binary);
		if (file) {
			std::ostringstream ss;
			ss << file.rdbuf();
			body = ss.str();
		} else {
			body = "<h1>html file not found</h1>";
		}
	}
	const std::string resp = build_response(body, ctype);
	send_all(client_fd, resp);
	close(client_fd);
}
