#ifndef CGI_SPAWN_HPP
#define CGI_SPAWN_HPP

#include <vector>
#include <string>
#include "../../include/webserv.hpp"

void log_env_connstate(const ConnState &st);
void spawn_cgi(ConnState &st);

#endif
