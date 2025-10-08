#include "../include/webserv.hpp"
#include "../include/configuration/configParse.hpp"
#include <arpa/inet.h>

//set socket to non-blocking
static void set_nonblocking(int fd) {
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
		throw std::runtime_error("non-blocking failed");
}

// link sockets in single server variable
int setup_server(int port, const ServerFlat& s) {
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1) throw std::runtime_error("socket failed");

	struct sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	if (s.host.empty())
		addr.sin_addr.s_addr = INADDR_ANY;
	else
		addr.sin_addr.s_addr = inet_addr(s.host.c_str());
	addr.sin_port = htons(port);

	set_nonblocking(server_fd);

	int yes = 1;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

	if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) == -1) {
		::close(server_fd);
		throw std::runtime_error("bind failed");
	}
	if (listen(server_fd, 128) == -1)
		throw std::runtime_error("listen failed");

	return server_fd;
}

int accept_client(int server_fd) {
    int cfd = accept(server_fd, NULL, NULL);
    if (cfd >= 0) {
        set_nonblocking(cfd);
        return cfd;
    }
    // On error, return -1 and let poll() tell us when to retry
    // No errno checking allowed per subject requirements
    return -1;
}

