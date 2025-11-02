#ifndef WEBSERV_HPP
#define WEBSERV_HPP

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
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctime>
#include <set>
#include <poll.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define CGI_TIMEOUT_MS 10000


enum { ACT_READ = 1, ACT_WRITE = 2, ACT_CLOSE = 4 };


struct ServerFlat;

enum ConnPhase { IDLE, CGI_SPAWN, CGI_STREAM };
struct ConnState {
    std::string in;
    std::string out;
    size_t      off;
    bool        resp_ready;
    time_t      last_activity;
    std::string host_header;
    const std::vector<size_t>* vhost_candidates;
    const std::vector<ServerFlat>* servers_all;

    std::string cgi_script;
    std::string cgi_interpreter;
    std::string cgi_cwd;
    pid_t cgi_pid;
    int   cgi_in;
    int   cgi_out;
    bool  cgi_in_open;
    bool  cgi_out_open;
    time_t cgi_start_time;
    bool  is_cgi_running;

    std::string cgi_raw;

    size_t      body_expected;
    size_t      body_received;
    bool        chunked;
    bool        body_done;
    std::string body_buf;
    size_t      cgi_written;
    bool        expect_continue;
    bool        reading_body;
    std::vector<std::string> env;

    ConnPhase   phase;
    int         client_fd;

    ConnState()
        : in(), out(), off(0), resp_ready(false),
          last_activity(time(NULL)), host_header(),
          vhost_candidates(NULL), servers_all(NULL),
          cgi_script(), cgi_interpreter(), cgi_cwd(),
          cgi_pid(-1), cgi_in(-1), cgi_out(-1),
          cgi_in_open(false), cgi_out_open(false),
          cgi_start_time(0), is_cgi_running(false),
          cgi_raw(),
          body_expected(0), body_received(0),
          chunked(false), body_done(true),
          body_buf(), cgi_written(0), expect_continue(false),
          reading_body(false), phase(IDLE),
          client_fd(-1)
    {}
};


class configParse;

struct ServerFlat;
struct Location;

int setup_server(int port, const ServerFlat& s);
int accept_client(int server_fd);
void handle_client(int client_fd, const ServerFlat& server);
bool getline_fd(int fd, std::string& out);
void spawn_cgi(ConnState& st);
void build_http_from_cgi(ConnState& st);

#endif
