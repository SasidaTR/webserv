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
#include <arpa/inet.h>
#include <fcntl.h>
#include <cstring>
#include <ctime>

int  handle_client(int fd, short revents, const ServerFlat& s, ConnState& st);


int accept_client(int server_fd) {
    int cfd = accept(server_fd, NULL, NULL);
    if (cfd >= 0) return cfd;
    return -1;
}

struct ListenerKey {
    std::string host;
    int         port;

    ListenerKey() : port(0) {}
    ListenerKey(const std::string& h, int p) : host(h), port(p) {}

    bool operator<(const ListenerKey& o) const {
        if (port != o.port) return port < o.port;
        return host < o.host;
    }
};

static inline std::string norm_host(const std::string& h) {
    return h.empty() ? std::string("0.0.0.0") : h;
}

static void set_events(std::vector<struct pollfd> &fds, int fd, short events) {
    for (size_t k = 0; k < fds.size(); ++k) {
        if (fds[k].fd == fd) { fds[k].events = events; return; }
    }
}

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

    int flags = fcntl(server_fd, F_GETFL, 0);
    if (flags == -1 || fcntl(server_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        ::close(server_fd);
        throw std::runtime_error("non-blocking failed");
    }

    int yes = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        ::close(server_fd);
        throw std::runtime_error("setsockopt failed");
    }

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) == -1) {
        ::close(server_fd);
        throw std::runtime_error("bind failed");
    }
    if (listen(server_fd, 128) == -1) {
        ::close(server_fd);
        throw std::runtime_error("listen failed");
    }

    return server_fd;
}


int main(int argc, char **argv) {
    const char *config_file = "./basic.config";
    if (argc > 2) {
        std::cerr << "Usage: ./webserv <config>\n";
        return 1;
    }

    std::vector<int> listen_fds;
    std::set<int>    is_listener;
    std::map<int, std::vector<size_t> > listen_owner;

    std::map<int, int> client_owner;

    std::map<int, ConnState> conns;

    try {
        configParse cfg((argc == 2) ? argv[1] : config_file);
        std::vector<ServerFlat> servers = cfg.getServers();

        std::signal(SIGPIPE, SIG_IGN);

        std::set<int> ports_with_any;
        for (size_t i = 0; i < servers.size(); ++i) {
            const std::string host = norm_host(servers[i].host);
            const int         port = std::atoi(servers[i].port.c_str());
            if (host == "0.0.0.0") ports_with_any.insert(port);
        }

        std::map<ListenerKey, int>          lfd_by_key;   // (host,port) -> fd
        std::map<int, std::vector<size_t> > vhosts_by_fd; // fd -> server indices

        for (size_t i = 0; i < servers.size(); ++i) {
            const int port = std::atoi(servers[i].port.c_str());
            const std::string chosen_host = ports_with_any.count(port)
                                            ? "0.0.0.0"
                                            : norm_host(servers[i].host);

            ListenerKey key(chosen_host, port);

            if (!lfd_by_key.count(key)) {
                ServerFlat tmp = servers[i];
                tmp.host = chosen_host;

                int lfd = setup_server(port, tmp);
                if (lfd == -1) throw std::runtime_error("setup_server failed");

                lfd_by_key[key] = lfd;

                listen_fds.push_back(lfd);
                is_listener.insert(lfd);

                std::cout << "Server listening at: http://" << key.host << ":" << port << "/\n";
            }

            int lfd = lfd_by_key[key];
            vhosts_by_fd[lfd].push_back(i);
        }
        listen_owner = vhosts_by_fd;

        std::vector<pollfd> fds;
        fds.reserve(listen_fds.size());
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
            for (std::map<int, ConnState>::iterator it = conns.begin(); it != conns.end();) {
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
                const int   fd = fds[j].fd;
                const short ev = fds[j].revents;
                if (!ev) continue;
                if (!is_listener.count(fd)) continue;

                if (ev & POLLIN) {
                    for (;;) {
                        int cfd = accept_client(fd);
                        if (cfd == -1) break;

                        int fl = fcntl(cfd, F_GETFL, 0);
                        if (fl != -1) fcntl(cfd, F_SETFL, fl | O_NONBLOCK);

                        struct pollfd np;
                        np.fd = cfd;
                        np.events = POLLIN;
                        np.revents = 0;
                        to_add.push_back(np);

                        client_owner[cfd] = fd;      // client -> listener fd
                        conns[cfd] = ConnState();    // init per-connection state
                        conns[cfd].last_activity = time(NULL);
                    }
                }
            }
            if (!to_add.empty()) 
                fds.insert(fds.end(), to_add.begin(), to_add.end());

            for (size_t i = 0; i < fds.size();) {
                const int   fd = fds[i].fd;
                const short ev = fds[i].revents;

                if (is_listener.count(fd) || !ev) { ++i; continue; }

                std::map<int,int>::iterator it = client_owner.find(fd);
                if (it == client_owner.end()) {
                    close(fd);
                    fds[i] = fds.back(); fds.pop_back();
                    continue;
                }

                int lfd = it->second;
                const std::vector<size_t>& cand = listen_owner[lfd];
                if (cand.empty()) {
                    close(fd);
                    conns.erase(fd);
                    client_owner.erase(it);
                    fds[i] = fds.back(); fds.pop_back();
                    continue;
                }
                size_t server_idx = cand[0];

                ConnState &st = conns[fd];
                int act = handle_client(fd, ev, servers[server_idx], st);

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

        for (size_t i = 0; i < listen_fds.size(); ++i) close(listen_fds[i]);

    } catch (const std::exception& e) {
        for (size_t i = 0; i < listen_fds.size(); ++i) close(listen_fds[i]);
        std::cerr << "Fatal: " << e.what() << "\n";
        return 1;
    } catch (...) {
        for (size_t i = 0; i < listen_fds.size(); ++i) close(listen_fds[i]);
        std::cerr << "Fatal: unknown error\n";
        return 1;
    }

    return 0;
}



