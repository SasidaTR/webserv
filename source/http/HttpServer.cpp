#include "../../include/http/HttpServer.hpp"
#include "../../include/http/Request.hpp"
#include "../../include/http/Response.hpp"
#include "../../include/http/Router.hpp"
#include <iostream>
#include <string>
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
	std::string raw_req;
	if (!recv_request_once(client_fd, raw_req)) {
		close(client_fd);
		return;
	}

	Request req;
	Response resp;

	if (!req.parse(raw_req)) {
		resp.setStatus(400);
		resp.setErrorBody(400);
		send_all(client_fd, resp.build());
		close(client_fd);
		return;
	}

	req.debugPrint();

	Router router(s);
	resp = router.route(req);

	send_all(client_fd, resp.build());
	close(client_fd);
}
