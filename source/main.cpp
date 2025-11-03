/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jvittoz <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/02 19:49:00 by jvittoz           #+#    #+#             */
/*   Updated: 2025/11/02 19:49:04 by jvittoz          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/webserv.hpp"
#include "../include/configuration/configParse.hpp"


static volatile bool g_stop_listen = false;

static void handle_sigint(int) {
    g_stop_listen = true;
    std::cerr << "\n[signal] SIGINT caught → closing listeners and waiting...\n";
}


enum FdKind { FD_LISTENER, FD_CLIENT, FD_CGI_IN, FD_CGI_OUT };
struct FdOwner { FdKind kind; int client_fd; };
static std::map<int, FdOwner> g_owner;

int  handle_client(int fd, short revents, const ServerFlat& s, ConnState& st);
void spawn_cgi(ConnState& st);
void build_http_from_cgi(ConnState& st);



static inline void add_pfd(std::vector<pollfd>& fds, int fd, short ev) {
    struct pollfd p; p.fd = fd; p.events = ev; p.revents = 0; fds.push_back(p);
}
static inline void set_events(std::vector<pollfd>& fds, int fd, short ev) {
    for (size_t i = 0; i < fds.size(); ++i) if (fds[i].fd == fd) { fds[i].events = ev; return; }
}
static inline void remove_fd(std::vector<pollfd>& fds, int fd) {
    for (size_t i = 0; i < fds.size(); ++i) if (fds[i].fd == fd) { fds[i] = fds.back(); fds.pop_back(); return; }
}



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

    std::signal(SIGPIPE, SIG_IGN);
    std::signal(SIGINT, handle_sigint);

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

        std::map<ListenerKey, int>          lfd_by_key;
        std::map<int, std::vector<size_t> > vhosts_by_fd;
        for (size_t i = 0; i < servers.size(); ++i) {
            const int port = std::atoi(servers[i].port.c_str());
            const std::string chosen_host =
                ports_with_any.count(port) ? "0.0.0.0" : norm_host(servers[i].host);

            ListenerKey key(chosen_host, port);

            if (lfd_by_key.count(key)) {
                int lfd = lfd_by_key[key];
                bool duplicate_name = false;

                const std::vector<size_t>& existing = vhosts_by_fd[lfd];
                for (size_t j = 0; j < existing.size(); ++j) {
                    size_t idx = existing[j];
                    if (servers[idx].name == servers[i].name) {
                        duplicate_name = true;
                        break;
                    }
                }

                if (duplicate_name) {
                    std::cerr << "Error: duplicate server on "
                              << chosen_host << ":" << port
                              << " (server_name=" << servers[i].name << ")\n";
                    throw std::runtime_error("duplicate server_name on same host:port");
                }

                vhosts_by_fd[lfd].push_back(i);
                std::cout << "[merge] " << key.host << ":" << port
                          << " added " << servers[i].name << std::endl;
                continue;
            }

            ServerFlat tmp = servers[i];
            tmp.host = chosen_host;

            int lfd = setup_server(port, tmp);
            if (lfd == -1)
                throw std::runtime_error("setup_server failed");

            g_owner[lfd] = (FdOwner){ FD_LISTENER, -1 };
            lfd_by_key[key] = lfd;

            listen_fds.push_back(lfd);
            is_listener.insert(lfd);
            vhosts_by_fd[lfd].push_back(i);

            std::cout << "[bind] http://" << key.host << ":" << port
                      << "/ (server_name=" << servers[i].name << ")\n";
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

            if (g_stop_listen && !listen_fds.empty()) {
                std::cerr << "Closing all listeners...\n";
                for (size_t i = 0; i < listen_fds.size(); ++i) {
                    close(listen_fds[i]);
                    is_listener.erase(listen_fds[i]);
                    remove_fd(fds, listen_fds[i]);
                    g_owner.erase(listen_fds[i]);
                }
                listen_fds.clear();
                break;
            }

            if (ret < 0) {
                if (errno == EINTR) continue;
                throw std::runtime_error("poll() failed");
            }

            time_t now = time(NULL);
            for (std::map<int, ConnState>::iterator it = conns.begin(); it != conns.end();) {
                ConnState &st = it->second;

                if (now - st.last_activity > 80) {
                    std::cout << "Timeout: closing connection fd=" << it->first << "\n";
                    close(it->first);
                    client_owner.erase(it->first);
                    remove_fd(fds, it->first);
                    g_owner.erase(it->first);

                    if (st.cgi_in_open)  { close(st.cgi_in);  g_owner.erase(st.cgi_in);  remove_fd(fds, st.cgi_in); }
                    if (st.cgi_out_open) { close(st.cgi_out); g_owner.erase(st.cgi_out); remove_fd(fds, st.cgi_out); }

                    conns.erase(it++);
                    continue;
                }

                if (st.is_cgi_running && (now - st.cgi_start_time > 5)) {
                    std::cerr << "[CGI TIMEOUT] killing pid=" << st.cgi_pid
                              << " after " << (now - st.cgi_start_time) << "s\n";
                    kill(st.cgi_pid, SIGKILL);
                    waitpid(st.cgi_pid, NULL, 0);
                    st.is_cgi_running = false;

                    if (st.cgi_in_open)  { close(st.cgi_in);  st.cgi_in_open  = false; g_owner.erase(st.cgi_in);  remove_fd(fds, st.cgi_in); }
                    if (st.cgi_out_open) { close(st.cgi_out); st.cgi_out_open = false; g_owner.erase(st.cgi_out); remove_fd(fds, st.cgi_out); }

                    st.out =
                        "HTTP/1.1 504 Gateway Timeout\r\n"
                        "Content-Type: text/plain\r\n"
                        "Content-Length: 25\r\n"
                        "Connection: close\r\n\r\n"
                        "CGI process took too long.\n";

                    st.resp_ready = true;
                    set_events(fds, it->first, POLLOUT);
                }

                ++it;
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

                        client_owner[cfd] = fd;
                        conns[cfd] = ConnState();
                        conns[cfd].last_activity = time(NULL);
                        g_owner[cfd] = (FdOwner){ FD_CLIENT, cfd };
                        conns[cfd].client_fd = cfd;
                    }
                }
            }
            if (!to_add.empty())
                fds.insert(fds.end(), to_add.begin(), to_add.end());

            for (size_t i = 0; i < fds.size(); ) {
                const int   fd = fds[i].fd;
                const short ev = fds[i].revents;

                if (is_listener.count(fd) || !ev) { ++i; continue; }

                FdOwner ow = g_owner.count(fd) ? g_owner[fd] : (FdOwner){FD_CLIENT, fd};

                if (ow.kind == FD_CLIENT) {
                    std::map<int,int>::iterator it = client_owner.find(fd);
                    if (it == client_owner.end()) {
                        close(fd);
                        g_owner.erase(fd);
                        fds[i] = fds.back(); fds.pop_back();
                        continue;
                    }

                    int lfd = it->second;
                    const std::vector<size_t>& cand = listen_owner[lfd];
                    if (cand.empty()) {
                        close(fd);
                        g_owner.erase(fd);
                        conns.erase(fd);
                        client_owner.erase(it);
                        fds[i] = fds.back(); fds.pop_back();
                        continue;
                    }
                    ConnState &st = conns[fd];
                    st.vhost_candidates = &cand;
                    st.servers_all = &servers;
                    size_t server_idx = cand[0];

                    if (ev & (POLLHUP | POLLERR | POLLNVAL)) {
                        if (st.cgi_in_open)  { close(st.cgi_in);  g_owner.erase(st.cgi_in);  remove_fd(fds, st.cgi_in); }
                        if (st.cgi_out_open) { close(st.cgi_out); g_owner.erase(st.cgi_out); remove_fd(fds, st.cgi_out); }
                        close(fd);
                        g_owner.erase(fd);
                        conns.erase(fd);
                        client_owner.erase(it);
                        fds[i] = fds.back(); fds.pop_back();
                        continue;
                    }

                    if (st.phase == CGI_STREAM && (ev & POLLIN)) {
                        char buf[1<<16];
                        ssize_t n = read(fd, buf, sizeof(buf));
                        if (n > 0) {
                            st.body_buf.append(buf, (size_t)n);
                            st.body_received += (size_t)n;
                            if (!st.chunked && st.body_expected && st.body_received >= st.body_expected)
                                st.body_done = true;
                            st.last_activity = time(NULL);
                            if (st.cgi_in_open) set_events(fds, st.cgi_in, POLLOUT);
                        } else if (n == 0) {
                            st.body_done = true;
                        }
                    }

                    int act = handle_client(fd, ev, servers[server_idx], st);

                    if (act & ACT_CLOSE) {
                        if (st.cgi_in_open)  { close(st.cgi_in);  g_owner.erase(st.cgi_in);  remove_fd(fds, st.cgi_in); }
                        if (st.cgi_out_open) { close(st.cgi_out); g_owner.erase(st.cgi_out); remove_fd(fds, st.cgi_out); }
                        close(fd);
                        g_owner.erase(fd);
                        conns.erase(fd);
                        client_owner.erase(it);
                        fds[i] = fds.back(); fds.pop_back();
                        continue;
                    }

                    if (st.phase == CGI_SPAWN && !st.body_done) {
                        set_events(fds, fd, POLLIN);
                        ++i;
                        continue;
                    }

                    if (st.phase == CGI_SPAWN && st.body_done) {
                        spawn_cgi(st);
                        if (st.cgi_in_open)  { add_pfd(fds, st.cgi_in,  POLLOUT); g_owner[st.cgi_in]  = (FdOwner){FD_CGI_IN,  fd}; }
                        if (st.cgi_out_open) { add_pfd(fds, st.cgi_out, POLLIN);  g_owner[st.cgi_out] = (FdOwner){FD_CGI_OUT, fd}; }
                        st.phase = CGI_STREAM;
                    }

                    short next = 0;
                    if (act & ACT_READ)  next |= POLLIN;
                    if (act & ACT_WRITE) next |= POLLOUT;
                    if (next == 0) next = POLLIN;
                    set_events(fds, fd, next);

                    ++i;
                    continue;
                }

                if (ow.kind == FD_CGI_IN) {
                    ConnState &st = conns[ow.client_fd];

                    if (ev & (POLLERR | POLLHUP | POLLNVAL)) {
                        if (st.cgi_in_open) { close(st.cgi_in); st.cgi_in_open = false; }
                        g_owner.erase(fd); remove_fd(fds, fd);
                        continue;
                    }

                    if (ev & POLLOUT) {
                        size_t avail = (st.body_buf.size() > st.cgi_written)
                                     ? (st.body_buf.size() - st.cgi_written) : 0;
                        if (avail) {
                            ssize_t nwr = write(st.cgi_in, &st.body_buf[st.cgi_written], avail);
                            if (nwr > 0) st.cgi_written += (size_t)nwr;
                        }
                        bool drained = (st.cgi_written == st.body_buf.size());
                        if (st.cgi_in_open && st.body_done && drained) {
                            close(st.cgi_in); st.cgi_in_open = false;
                            g_owner.erase(fd); remove_fd(fds, fd);
                            continue;
                        }
                        if (st.cgi_in_open) set_events(fds, fd, POLLOUT);
                    }

                    ++i;
                    continue;
                }
                
                if (ow.kind == FD_CGI_OUT) {
                    ConnState &st = conns[ow.client_fd];

                    if (ev & POLLIN) {
                        char buf[1<<16];
                        for (;;) {
                            ssize_t nrd = read(st.cgi_out, buf, sizeof(buf));
                            if (nrd > 0) {
                                st.cgi_raw.append(buf, (size_t)nrd);
                                continue;
                            }
                            if (nrd == 0) {
                                close(st.cgi_out); st.cgi_out_open = false;
                            }
                            break;
                        }
                    }

                    if ((ev & (POLLHUP | POLLERR | POLLNVAL)) || !st.cgi_out_open) {
                        if (st.cgi_out_open) { close(st.cgi_out); st.cgi_out_open = false; }
                        g_owner.erase(fd);
                        remove_fd(fds, fd);

                        build_http_from_cgi(st);
                        st.off = 0;
                        st.resp_ready = true;
                        set_events(fds, ow.client_fd, POLLOUT);

                        int status = 0; (void)waitpid(st.cgi_pid, &status, WNOHANG);
                        continue;
                    }

                    if (st.cgi_out_open) set_events(fds, fd, POLLIN);

                    ++i;
                    continue;
                }
                close(fd);
                g_owner.erase(fd);
                fds[i] = fds.back(); fds.pop_back();
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


