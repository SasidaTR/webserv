#include "include/hpp/webserv.hpp"

int main(int argc, char **argv) {
	(void)argc;
	try {
		configParse config(argv[1]);
	} catch (const std::exception& e) {
		std::cerr << "Config error: " << e.what() << "\n";
		return 1;
	} catch (...) {
		std::cerr << "Unknown config error\n";
		return 1;
	}
	try {
		std::signal(SIGPIPE, SIG_IGN);

		int server_fd = setup_server(8080);

		std::vector<pollfd> fds;
		struct pollfd p;
		p.fd = server_fd;
		p.events = POLLIN;
		p.revents = 0;
		fds.push_back(p);

		while (true) {
			int ret = poll(&fds[0], (nfds_t)fds.size(), 1000);
			if (ret == -1)
				throw std::runtime_error("poll() failed");

			for (size_t i = 0; i < fds.size(); i++) {
				pollfd &ptr = fds[i];

				if (ptr.revents == 0)
					continue;

				if (ptr.fd == server_fd && (ptr.revents & POLLIN)) {
					int client_fd = accept_client(server_fd);
					if (client_fd != -1) {
						struct pollfd np;
						np.fd = client_fd;
						np.events = POLLIN;
						np.revents = 0;
						fds.push_back(np);
					}
					continue;
				}

				if (ptr.revents & POLLIN) {
					handle_client(ptr.fd);
					fds[i] = fds.back();
					fds.pop_back();
					--i;
					continue;
				}

				if (ptr.revents & (POLLHUP | POLLERR | POLLNVAL)) {
					close(ptr.fd);
					fds[i] = fds.back();
					fds.pop_back();
					--i;
				}
			}
		}

		close(server_fd);
	} catch (const std::exception& e) {
		std::cerr << "Fatal: " << e.what() << "\n";
		return 1;
	}

	return 0;
}
