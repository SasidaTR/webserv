#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP

#include "../configuration/configParse.hpp"

class Request;
class Response;
class Router;

void handle_client(int client_fd, const ServerFlat& s);

#endif
