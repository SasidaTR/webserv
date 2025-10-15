#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP

#include "../configuration/configParse.hpp"
#include <poll.h>

struct ConnState;

int handle_client(int fd, short revents, const ServerFlat& s, ConnState& st);

#endif