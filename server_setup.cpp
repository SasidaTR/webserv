#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <stdexcept>

int setup_server(int port) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) throw std::runtime_error("socket failed");

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;   // 0.0.0.0
    addr.sin_port = htons(port);

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) == -1)
        throw std::runtime_error("bind failed");

    if (listen(server_fd, 1) == -1)
        throw std::runtime_error("listen failed");

    std::cout << "Listening on http://localhost:" << port << "\n";

    return server_fd;
}

int accept_client(int server_fd) {
    int client_fd = accept(server_fd, NULL, NULL);
    if (client_fd == -1)
        throw std::runtime_error("accept failed");
    return client_fd;
}
