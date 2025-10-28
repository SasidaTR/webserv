#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cerrno>
#include <cstring>
#include <sstream>
#include "../../include/webserv.hpp"
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <iostream>
#include <cstdio>



#define LOGCGI(fmt, ...) do { \
    fprintf(stderr, "[CGI] " fmt "\n", ##__VA_ARGS__); \
} while (0)

static inline const char* nz(const char* p) { return p ? p : "<NULL>"; }

static void log_env_kv(const char* k) {
    const char* v = getenv(k);
    LOGCGI("env %s=%s", k, nz(v));
}

// ---- tiny helper: set non-blocking on an fd ----
static inline void set_nonblock_fd(int fd) {
    int fl = fcntl(fd, F_GETFL, 0);
    if (fl != -1) fcntl(fd, F_SETFL, fl | O_NONBLOCK);
}

void log_env_connstate(const ConnState &st) {
    LOGCGI("=== ConnState environment (%zu entries) ===", st.env.size());
    for (size_t i = 0; i < st.env.size(); ++i) {
        LOGCGI("env %s", st.env[i].c_str());
    }
    LOGCGI("=== End of ConnState environment ===");
}


// ---- spawn_cgi: fork/exec interpreter + script, set up pipes ----
void spawn_cgi(ConnState& st) {
    int pin[2];     // parent writes  -> child stdin
    int pout[2];    // child writes   -> parent reads

    LOGCGI("spawn requested: interp='%s' script='%s'",
           st.cgi_interpreter.c_str(), st.cgi_script.c_str());

    if (access(st.cgi_interpreter.c_str(), X_OK) != 0) {
        LOGCGI("WARNING: interpreter not executable or not found (errno=%d: %s)",
               errno, strerror(errno));
    }
    if (access(st.cgi_script.c_str(), R_OK) != 0) {
        LOGCGI("WARNING: script not readable (errno=%d: %s)",
               errno, strerror(errno));
    }

    log_env_connstate(st);

    LOGCGI("checking expected CGI env vars:");
    log_env_kv("GATEWAY_INTERFACE");
    log_env_kv("REQUEST_METHOD");
    log_env_kv("SERVER_PROTOCOL");
    log_env_kv("SERVER_NAME");
    log_env_kv("SERVER_PORT");
    log_env_kv("SCRIPT_NAME");
    log_env_kv("SCRIPT_FILENAME");
    log_env_kv("QUERY_STRING");
    log_env_kv("REMOTE_ADDR");
    log_env_kv("PATH_INFO");
    log_env_kv("PATH_TRANSLATED");
    log_env_kv("CONTENT_TYPE");
    log_env_kv("CONTENT_LENGTH");
    log_env_kv("PATH");

    if (pipe(pin)  == -1) { LOGCGI("pipe(pin) failed: %s", strerror(errno)); throw std::runtime_error("pipe(pin) failed"); }
    if (pipe(pout) == -1) {
        LOGCGI("pipe(pout) failed: %s", strerror(errno));
        close(pin[0]); close(pin[1]);
        throw std::runtime_error("pipe(pout) failed");
    }

    pid_t pid = fork();
    if (pid < 0) {
        LOGCGI("fork() failed: %s", strerror(errno));
        close(pin[0]); close(pin[1]); close(pout[0]); close(pout[1]);
        throw std::runtime_error("fork() failed");
    }

    if (pid == 0) {
        dup2(pin[0], 0);
        dup2(pout[1], 1);
        dup2(pout[1], 2);
        close(pin[0]); close(pin[1]);
        close(pout[0]); close(pout[1]);

        std::vector<char*> argv;
        if (!st.cgi_interpreter.empty()) {
            argv.push_back(const_cast<char*>(st.cgi_interpreter.c_str()));
            argv.push_back(const_cast<char*>(st.cgi_script.c_str()));
        } else {
            argv.push_back(const_cast<char*>(st.cgi_script.c_str()));
        }
        argv.push_back(NULL);

        std::vector<char*> envp;
        envp.reserve(st.env.size() + 1);
        for (size_t i = 0; i < st.env.size(); ++i)
            envp.push_back(const_cast<char*>(st.env[i].c_str()));
        envp.push_back(NULL);

        for (size_t i = 0; i < st.env.size(); ++i)
           LOGCGI("env %s", st.env[i].c_str());

        log_env_connstate(st);


        execve(argv[0], &argv[0], &envp[0]);


        const char* msg =
            "Status: 500 Internal Server Error\r\n"
            "Content-Type: text/plain\r\n\r\nexecve failed\n";
        write(1, msg, std::strlen(msg));
        _exit(127);
    }

    // ---- parent process ----
    st.cgi_pid = pid;

    st.cgi_in  = pin[1];  close(pin[0]);
    st.cgi_out = pout[0]; close(pout[1]);

    set_nonblock_fd(st.cgi_in);
    set_nonblock_fd(st.cgi_out);

    st.cgi_in_open  = true;
    st.cgi_out_open = true;

    LOGCGI("spawned pid=%d, cgi_in(fd)=%d, cgi_out(fd)=%d", (int)pid, st.cgi_in, st.cgi_out);
    LOGCGI("body_expected=%zu chunked=%s body_done=%s",
           (size_t)st.body_expected, st.chunked ? "true" : "false", st.body_done ? "true" : "false");
}


// ---- small helper to map status code -> reason phrase ----
static std::string http_reason_from_code(int code) {
    switch (code) {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 302: return "Found";
        case 400: return "Bad Request";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 413: return "Payload Too Large";
        case 500: return "Internal Server Error";
        case 502: return "Bad Gateway";
        default:  return "OK";
    }
}

// ---- build_http_from_cgi: turn st.cgi_raw into a full HTTP/1.1 response ----
void build_http_from_cgi(ConnState& st) {
    const std::string& raw = st.cgi_raw;
    const size_t sep = raw.find("\r\n\r\n");

    if (sep == std::string::npos) {
        // no CGI header block → synthesize a 502
        const std::string body = "Bad Gateway: CGI produced no headers\n";
        std::ostringstream os;
        os << "HTTP/1.1 502 Bad Gateway\r\n"
           << "Content-Type: text/plain\r\n"
           << "Content-Length: " << body.size() << "\r\n"
           << "Connection: close\r\n\r\n"
           << body;
        st.out = os.str();
        return;
    }

    std::string head = raw.substr(0, sep);
    std::string body = raw.substr(sep + 4);

    // parse CGI-style headers (CRLF-separated "Key: Value")
    std::istringstream hs(head);
    std::string line;
    int status_code = 200;
    std::vector<std::pair<std::string,std::string> > hdrs;

    while (std::getline(hs, line)) {
        if (!line.empty() && line[line.size()-1] == '\r') line.erase(line.size()-1);
        size_t colon = line.find(':');
        if (colon == std::string::npos) continue;

        std::string key = line.substr(0, colon);
        std::string val = line.substr(colon + 1);
        while (!val.empty() && (val[0] == ' ' || val[0] == '\t')) val.erase(0,1);

        // "Status: 200 OK"
        if (key == "Status" || key == "Status:") {
            std::istringstream ss(val);
            int c = 0; ss >> c;
            if (c > 0) status_code = c;
        } else {
            hdrs.push_back(std::make_pair(key, val));
        }
    }

    bool has_cl = false, has_conn = false;
    for (size_t i = 0; i < hdrs.size(); ++i) {
        std::string k = hdrs[i].first;
        for (size_t j=0;j<k.size();++j) if (k[j]>='A' && k[j]<='Z') k[j] = char(k[j]-'A'+'a');
        if (k == "content-length") has_cl = true;
        if (k == "connection")     has_conn = true;
    }

    std::ostringstream os;
    os << "HTTP/1.1 " << status_code << " " << http_reason_from_code(status_code) << "\r\n";
    for (size_t i = 0; i < hdrs.size(); ++i) {
        os << hdrs[i].first << ": " << hdrs[i].second << "\r\n";
    }
    if (!has_cl)   os << "Content-Length: " << body.size() << "\r\n";
    if (!has_conn) os << "Connection: close\r\n";
    os << "\r\n" << body;

    st.out = os.str();
}
