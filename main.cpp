#include "./source/http_handler.cpp"
#include "./source/server_setup.cpp"
#include <csignal>
#include <vector>
#include <poll.h>
#include <unistd.h>
#include <iostream>
#include <stdexcept>


int main() {
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
            int ret = poll(fds.data(), fds.size(), 1000); // 1s timeout
            if (ret == -1)
                throw std::runtime_error("poll() failed");

            for (size_t i = 0; i < fds.size(); i++) {
                pollfd &ptr = fds[i];

                if (ptr.revents == 0)
                    continue;

                // New client
                if (ptr.fd == server_fd && (ptr.revents & POLLIN)) {
                    int client_fd = accept(server_fd, NULL, NULL);
                    if (client_fd != -1) {
                        struct pollfd np;
                        np.fd = client_fd;
                        np.events = POLLIN;
                        np.revents = 0;
                        fds.push_back(np);
                    }
                    continue;
                }

                // Existing client ready to read
                if (ptr.revents & POLLIN) {
                    handle_client(ptr.fd);

                    // remove fd from list
                    fds[i] = fds.back();
                    fds.pop_back();
                    --i;
                    continue;
                }

                // Clean up if error/hangup
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
