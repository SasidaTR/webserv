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
#include <ctime>

enum { ACT_READ = 1, ACT_WRITE = 2, ACT_CLOSE = 4 };

struct ConnState {
    std::string in, out;
    size_t      off;
    bool        resp_ready;
    time_t      last_activity;
    ConnState() : off(0), resp_ready(false), last_activity(time(NULL)) {}
};

class configParse;

struct ServerFlat;
struct Location;

int setup_server(int port, const ServerFlat& s);
int accept_client(int server_fd);
void handle_client(int client_fd, const ServerFlat& server);
bool getline_fd(int fd, std::string& out);

#endif
