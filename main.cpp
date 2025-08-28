#include "./source/http_handler.cpp"
#include "./source/server_setup.cpp"
#include <csignal>
#include <vector>
#include <poll.h>
#include <unistd.h>
#include <iostream>
#include <stdexcept>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <sys/socket.h>

static void add_fd(std::vector<struct pollfd> &fds, int fd, short events) {
    struct pollfd p;
    p.fd = fd;
    p.events = events;
    p.revents = 0;
    fds.push_back(p);
}

int main() {
    try {
        std::signal(SIGPIPE, SIG_IGN);

        int server_fd = setup_server(8080);

        std::vector<struct pollfd> fds;
        add_fd(fds, server_fd, POLLIN);  // <- CRUCIAL: watch the listening socket

        while (true) {
            int ready = poll(&fds[0], fds.size(), -1);
            if (ready < 0) {
                if (errno == EINTR) continue;
                std::perror("poll");
                break;
            }

            for (size_t i = 0; i < fds.size(); ++i) {
                struct pollfd &p = fds[i];

                // New connections
                if (p.fd == server_fd && (p.revents & POLLIN)) {
                    for (;;) {
                        int client_fd = accept(server_fd, 0, 0);
                        if (client_fd < 0) {
                            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                            std::perror("accept");
                            break;
                        }
                        // Non-blocking client is optional because handle_client() reads once and closes.
                        // If you prefer non-blocking:
                        // int flags = fcntl(client_fd, F_GETFL, 0);
                        // fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);
                        add_fd(fds, client_fd, POLLIN);
                    }
                }

                // Client data
                else if (p.fd != server_fd && (p.revents & POLLIN)) {
                    // handle_client() does a single recv(), builds response, send(), close()
                    handle_client(p.fd);

                    // Remove the fd from the vector (swap-with-back)
                    close(p.fd); // safe even though handle_client closed; ensures cleanup
                    fds[i] = fds.back();
                    fds.pop_back();
                    --i; // adjust index since we removed current element
                }

                // Errors/HUPs on clients
                else if (p.fd != server_fd && (p.revents & (POLLERR | POLLHUP | POLLNVAL))) {
                    close(p.fd);
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
