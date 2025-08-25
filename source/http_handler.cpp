#include <string>
#include <unistd.h>
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
