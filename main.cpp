#include "./include/webserv.hpp"
#include "./include/configuration/configParse.hpp" 
#include <set>
#include <map>
#include <csignal>
#include <iostream>
#include <vector>
#include <stdexcept>

int main(int argc, char **argv) {
	if (argc != 2) {
		std::cerr << "Usage: ./webserv <config>\n";
		return 1;
	}

	std::vector<int> listen_fds;
	std::set<int>    is_listener;
	std::map<int, size_t> listen_owner;
	std::map<int, size_t> client_owner;

	try {
		// setup server info;
		configParse cfg(argv[1]);
		std::vector<ServerFlat> servers = cfg.getServers(); 

		std::signal(SIGPIPE, SIG_IGN);

		// setup open socket;
		for (size_t i = 0; i < servers.size(); ++i) {
			int lfd = setup_server(atoi(servers[i].port.c_str()), servers[i]); 
			if (lfd == -1)
				throw std::runtime_error("setup_server failed");

			listen_fds.push_back(lfd);
			is_listener.insert(lfd);
			listen_owner[lfd] = i;

			std::string host = servers[i].host.empty() ? "0.0.0.0" : servers[i].host;
			std::cout << "Server listening at: http://" << host << ":" << servers[i].port << "/" << std::endl;
		}

		// setup listning sockets;
		std::vector<pollfd> fds;
		for (size_t i = 0; i < listen_fds.size(); ++i) {
			struct pollfd p;
			p.fd = listen_fds[i];
			p.events = POLLIN;
			p.revents = 0;
			fds.push_back(p);
		}

		//main loop;
		while (true) {
			//poll call;
			int ret = poll(&fds[0], (nfds_t)fds.size(), 1000);
			if (ret < 0) {
				if (errno == EINTR) continue;
				throw std::runtime_error("poll() failed");
			}
			if (ret == 0) continue;


			// connect on listner
			std::vector<struct pollfd> to_add;
			for (size_t j = 0; j < fds.size(); ++j) {
				const int fd   = fds[j].fd;
				const short ev = fds[j].revents;

				if (!ev) continue;

				if (is_listener.count(fd)) {
					if (ev & POLLIN) {
						//go through accept list
						for (;;) {
							int cfd = accept_client(fd);
							if (cfd == -1) 
								break;
							struct pollfd np;
							np.fd = cfd;
							np.events = POLLIN;
							np.revents = 0;
							to_add.push_back(np);
							client_owner[cfd] = listen_owner[fd];
						}
					}
					if (ev & (POLLERR | POLLHUP | POLLNVAL)) {
					}
				}
			}
			// link clients to fd;
			if (!to_add.empty()) {
				fds.insert(fds.end(), to_add.begin(), to_add.end());
			}

			// manage client answer
			for (size_t i = 0; i < fds.size();) {
				const int fd   = fds[i].fd;
				const short ev = fds[i].revents;

				if (is_listener.count(fd) || !ev) { ++i; continue; }

				if (ev & POLLIN) {
					std::map<int, size_t>::iterator it = client_owner.find(fd);
					if (it == client_owner.end()) {
						close(fd);
						fds[i] = fds.back(); fds.pop_back();
						continue;
					}
					size_t idx = it->second;
					handle_client(fd, servers[idx]); 

					close(fd);
					client_owner.erase(it);
					fds[i] = fds.back(); fds.pop_back();
					continue;
				}
				if (ev & (POLLHUP | POLLERR | POLLNVAL)) {
					close(fd);
					client_owner.erase(fd);
					fds[i] = fds.back(); fds.pop_back();
					continue;
				}

				++i;
			}
		}

		for (size_t i = 0; i < listen_fds.size(); ++i)
			close(listen_fds[i]);

	} catch (const std::exception& e) {
		std::cerr << "Fatal: " << e.what() << "\n";
		return 1;
	} catch (...) {
		std::cerr << "Fatal: unknown error\n";
		return 1;
	}

	return 0;
}

