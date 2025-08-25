#include <iostream>
#include <unistd.h>
#include <string>
#include <unistd.h>
#include <stdexcept>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <stdexcept>

std::string build_response(const std::string& body) {
    return "HTTP/1.1 200 OK\r\n"
           "Content-Type: text/html\r\n"
           "Content-Length: " + std::to_string(body.size()) + "\r\n"
           "\r\n" +
           body;
}

void handle_client(int client_fd) {
    std::string body = "<h1>Hello Web</h1>";
    std::string response = build_response(body);

    if (send(client_fd, response.c_str(), response.size(), 0) < 0)
        throw std::runtime_error("send failed");

    close(client_fd);
}

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
