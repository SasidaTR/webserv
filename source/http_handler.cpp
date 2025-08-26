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
#include <fcntl.h>
#include <fstream>
#include <sstream>

std::string build_response(const std::string& body) {
    return "HTTP/1.1 200 OK\r\n"
           "Content-Type: text/html\r\n"
           "Content-Length: " + std::to_string(body.size()) + "\r\n"
           "\r\n" +
           body;
}

void handle_client(int client_fd) {
    // terminal response :
    std::cout << "Through the loop again..." << std::endl;

    // index.htlm handeling :
    std::ifstream file("./html/index.html");
    std::string body;
    if (file) {
        std::ostringstream ss;
        ss << file.rdbuf();
        body = ss.str();
    } else {
        body = "<h1>html file not found</h1>";
    }

    // response message
    std::string response = build_response(body);
    if (send(client_fd, response.c_str(), response.size(), 0) < 0)
        throw std::runtime_error("send failed");

    close(client_fd);
}
