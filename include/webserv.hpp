#ifndef WEBSERV_HPP
#define WEBSERV_HPP

// C++ headers
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <stdexcept>
#include <cstring>
#include <cerrno>
#include <csignal>
#include <cstdlib> 

// System headers
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

enum { ACT_READ = 1, ACT_WRITE = 2, ACT_CLOSE = 4 };

struct ConnState {
    std::string in, out;  // accumulated request, built response
    size_t      off;      // bytes already sent from 'out'
    bool        resp_ready;
    ConnState() : off(0), resp_ready(false) {}
};

class configParse;

struct ServerFlat;
struct Location;

int setup_server(int port, const ServerFlat& s);
int accept_client(int server_fd);
void handle_client(int client_fd, const ServerFlat& server);
bool getline_fd(int fd, std::string& out);

#endif
