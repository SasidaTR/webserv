#include "./source/http_handler.cpp"
#include "./source/server_setup.cpp"
#include <csignal>
#include <vector>


int main() {
    try {
        std::signal(SIGPIPE, SIG_IGN);
        int server_fd = setup_server(8080);
        std::vector<int> client_sockets;

        while (true) {
            while (true) {
                int client_fd = accept(server_fd, NULL, NULL);
                if (client_fd == -1) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) break; // no more pending clients
                    throw std::runtime_error("accept failed");
                }
                client_sockets.push_back(client_fd);
            }
            for (size_t i = 0; i < client_sockets.size();) {
                int fd = client_sockets[i];
                handle_client(fd);
                client_sockets[i] = client_sockets.back();
                client_sockets.pop_back();
            }
            usleep(100);
        }

        close(server_fd);
    } catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
