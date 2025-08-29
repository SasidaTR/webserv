#include "../include/webserv.hpp"

static void set_nonblocking(int fd) {
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
		throw std::runtime_error("non-blocking failed");
}

int setup_server(int port) {
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1) throw std::runtime_error("socket failed");

	struct sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);

	set_nonblocking(server_fd);

	int yes = 1;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

	if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) == -1)
		throw std::runtime_error("bind failed");
	if (listen(server_fd, 128) == -1)
		throw std::runtime_error("listen failed");

	return server_fd;
}

int accept_client(int server_fd) {
	int cfd = accept(server_fd, NULL, NULL);
	if (cfd == -1) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) return -1;
		throw std::runtime_error("accept failed");
	}
	set_nonblocking(cfd);
	return cfd;
}
