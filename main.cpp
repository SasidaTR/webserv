#include "./include/webserv.hpp"
#include "./include/configuration/configParse.hpp"
#include <set>
#include <map>
#include <csignal>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <poll.h>
#include <errno.h>
#include <unistd.h>

/*--> add data limit option to config*/


int  accept_client(int server_fd);
int  handle_client(int fd, short revents, const ServerFlat& s, ConnState& st);

static void set_events(std::vector<struct pollfd> &fds, int fd, short events) {
    for (size_t k = 0; k < fds.size(); ++k) {
        if (fds[k].fd == fd) { fds[k].events = events; return; }
    }
}

int main(int argc, char **argv) {
    char config_file[1024] = "./basic.config";
    if (argc > 2) {
        std::cerr << "Usage: ./webserv <config>\n";
        return 1;
    }

    std::vector<int> listen_fds;
    std::set<int>    is_listener;
    std::map<int, size_t> listen_owner; // listener fd -> server index
    std::map<int, size_t> client_owner; // client fd   -> server index
    std::map<int, ConnState> conns;     // client fd   -> per-connection state

    try {
        configParse cfg((argc == 2) ? argv[1] : config_file);
        std::vector<ServerFlat> servers = cfg.getServers();

        std::signal(SIGPIPE, SIG_IGN);

        for (size_t i = 0; i < servers.size(); ++i) {
            int lfd = setup_server(atoi(servers[i].port.c_str()), servers[i]);
            if (lfd == -1)
                throw std::runtime_error("setup_server failed");

            listen_fds.push_back(lfd);
            is_listener.insert(lfd);
            listen_owner[lfd] = i;

            std::string host = servers[i].host.empty() ? "0.0.0.0" : servers[i].host;
            std::cout << "Server listening at: http://" << host << ":" << servers[i].port << "/\n";
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
            if (fds.empty()) break;
            int ret = poll(&fds[0], (nfds_t)fds.size(), 1000);
            if (ret < 0) {
                if (errno == EINTR) continue;
                throw std::runtime_error("poll() failed");
            }
            
            time_t now = time(NULL);
            for (std::map<int, ConnState>::iterator it = conns.begin(); it != conns.end(); ) {
                if (now - it->second.last_activity > 60) {
                    std::cout << "Timeout: closing connection fd=" << it->first << "\n";
                    close(it->first);
                    client_owner.erase(it->first);
                    
                    for (size_t k = 0; k < fds.size(); ++k) {
                        if (fds[k].fd == it->first) {
                            fds[k] = fds.back();
                            fds.pop_back();
                            break;
                        }
                    }
                    
                    std::map<int, ConnState>::iterator tmp = it;
                    ++it;
                    conns.erase(tmp);
                } else {
                    ++it;
                }
            }
            
            if (ret == 0) continue;

            std::vector<struct pollfd> to_add;
            for (size_t j = 0; j < fds.size(); ++j) {
                const int fd   = fds[j].fd;
                const short ev = fds[j].revents;
                if (!ev) continue;
                if (!is_listener.count(fd)) continue;

                if (ev & POLLIN) {
                    for (;;) {
                        int cfd = accept_client(fd);
                        if (cfd == -1) break;

                        struct pollfd np;
                        np.fd = cfd;
                        np.events = POLLIN;
                        np.revents = 0;
                        to_add.push_back(np);

                        client_owner[cfd] = listen_owner[fd];
                        conns[cfd] = ConnState();
                    }
                }
            }
            if (!to_add.empty()) {
                fds.insert(fds.end(), to_add.begin(), to_add.end());
            }

            for (size_t i = 0; i < fds.size();) {
                const int   fd = fds[i].fd;
                const short ev = fds[i].revents;

                if (is_listener.count(fd) || !ev) { ++i; continue; }

                std::map<int,size_t>::iterator it = client_owner.find(fd);
                if (it == client_owner.end()) {
                    close(fd);
                    fds[i] = fds.back(); fds.pop_back();
                    continue;
                }

                ConnState &st = conns[fd];
                int act = handle_client(fd, ev, servers[it->second], st);

                if (act & ACT_CLOSE) {
                    close(fd);
                    conns.erase(fd);
                    client_owner.erase(it);
                    fds[i] = fds.back(); fds.pop_back();
                    continue;
                }

                short next = 0;
                if (act & ACT_READ)  next |= POLLIN;
                if (act & ACT_WRITE) next |= POLLOUT;
                if (next == 0) next = POLLIN;

                set_events(fds, fd, next);
                ++i;
            }
        }

        for (size_t i = 0; i < listen_fds.size(); ++i)
            close(listen_fds[i]);

    } catch (const std::exception& e) {
        for (size_t i = 0; i < listen_fds.size(); ++i)
            close(listen_fds[i]);
        std::cerr << "Fatal: " << e.what() << "\n";
        return 1;
    } catch (...) {
        for (size_t i = 0; i < listen_fds.size(); ++i)
            close(listen_fds[i]);
        std::cerr << "Fatal: unknown error\n";
        return 1;
    }

    return 0;
}


