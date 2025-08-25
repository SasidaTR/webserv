#include <iostream>
#include <unistd.h>
#include "server_setup.cpp"
#include "http_handler.cpp"

int main() {
    try {
        int server_fd = setup_server(8080);
        int client_fd = accept_client(server_fd);

        handle_client(client_fd);

        close(server_fd);
    } catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
