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

// System headers
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int setup_server(int port);
int accept_client(int server_fd);
void handle_client(int client_fd);

#endif
