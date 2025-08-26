#include "../include/webserv.hpp"

int setup_server(int port) {
	// open socket
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1) throw std::runtime_error("socket failed");

	// define socket parameter
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);

	// set non blocking and reopening
	int flags = fcntl(server_fd, F_GETFL, 0);
	if (flags == -1 || fcntl(server_fd, F_SETFL, flags | O_NONBLOCK) == -1)
		throw std::runtime_error("Could not define as non-blocking");
	int yes = 1;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

	// link socket
	if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) == -1)
		throw std::runtime_error("bind failed");
	if (listen(server_fd, 1) == -1)
		throw std::runtime_error("listen failed");

	std::cout << "Listening on http://localhost:" << port << "\n";
	return server_fd;
}

int accept_client(int server_fd) {
	int client_fd = accept(server_fd, NULL, NULL);
	if (client_fd == -1) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			return -1; // no client yet
		}
		throw std::runtime_error("accept failed");
	}
	return client_fd;
}
