#include "./include/webserv.hpp"
#include "./include/configuration/configParse.hpp" 
#include <set>
#include <map>
#include <csignal>
#include <iostream>
#include <vector>
#include <stdexcept>

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "Usage: ./webserv <config>\n";
        return 1;
    }

    std::vector<int> listen_fds;
    std::set<int>    is_listener;
    std::map<int, size_t> listen_owner;
    std::map<int, size_t> client_owner;

    try {
        configParse cfg(argv[1]);
        std::vector<ServerFlat> servers = cfg.getServers(); 

        std::signal(SIGPIPE, SIG_IGN);

        for (size_t i = 0; i < servers.size(); ++i) {
            int lfd = setup_server(atoi(servers[i].port.c_str()), servers[i]); 
            if (lfd == -1)
                throw std::runtime_error("setup_server failed");

            listen_fds.push_back(lfd);
            is_listener.insert(lfd);
            listen_owner[lfd] = i;
        }

        std::vector<pollfd> fds;
        for (size_t i = 0; i < listen_fds.size(); ++i) {
            struct pollfd p;
            p.fd = listen_fds[i];
            p.events = POLLIN;
            p.revents = 0;
            fds.push_back(p);
        }

        while (true) {
            int ret = poll(&fds[0], (nfds_t)fds.size(), 1000);
            if (ret == -1)
                throw std::runtime_error("poll() failed");

            for (size_t i = 0; i < fds.size(); ++i) {
                pollfd &ptr = fds[i];
                if (ptr.revents == 0)
                    continue;
                if (is_listener.count(ptr.fd) && (ptr.revents & POLLIN)) {
                    int client_fd = accept_client(ptr.fd);
                    if (client_fd != -1) {
                        struct pollfd np;
                        np.fd = client_fd;
                        np.events = POLLIN;
                        np.revents = 0;
                        fds.push_back(np);
                        client_owner[client_fd] = listen_owner[ptr.fd];
                    }
                    continue;
                }
                if (ptr.revents & POLLIN) {
                    size_t idx = client_owner[ptr.fd];
                    handle_client(ptr.fd, servers[idx]);

                    close(ptr.fd);
                    client_owner.erase(ptr.fd);
                    fds[i] = fds.back();
                    fds.pop_back();
                    --i;
                    continue;
                }
                if (ptr.revents & (POLLHUP | POLLERR | POLLNVAL)) {
                    close(ptr.fd);
                    client_owner.erase(ptr.fd);
                    fds[i] = fds.back();
                    fds.pop_back();
                    --i;
                }
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

