#ifndef WEBSERV_HPP
#define WEBSERV_HPP

int setup_server(int port);
int accept_client(int server_fd);
void handle_client(int client_fd);

#endif
