#include "../../include/http/HttpServer.hpp"
#include "../../include/http/Request.hpp"
#include "../../include/http/Response.hpp"
#include "../../include/http/Router.hpp"
#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <poll.h>
#include <ctime>


struct ServerFlat;

struct ConnState;

int atoi_b(char *str)
{
    int i = 0;
    int exp = 0;
    int res = 0;

    while (str[i] && str[i] >= '0' && str[i] <= '9')
    {
        res = res * 10 + (str[i] - 48);
        ++i;
    }
    if (!str[i]);
    else if (str[i] == 'm')
        exp = 1024*1024;
    else if (str[i] == 'k')
        exp = 1024*1024*1024;    
    else throw std::runtime_error("expects Xm or Xk as quantity");
    if (res == 0) throw std::runtime_error("0 not valid quantity");
    return res * exp;
}

static int try_recv_all_ready(int fd, std::string &buf) {
    char tmp[10000];
    for (;;) {
        ssize_t n = recv(fd, tmp, sizeof(tmp), 0);
        if (n > 0) { buf.append(tmp, n); continue; }
        if (n == 0) return -1;
        return 0;
    }
}

static int try_send_progress(int fd, const std::string &out, size_t &off) {
    while (off < out.size()) {
        ssize_t n = send(fd, out.data() + off, out.size() - off, 0);
        if (n > 0) { off += (size_t)n; continue; }
        if (n == 0) return -1;
        return 0;
    }
    return 1;
}

static bool headers_complete(const std::string &in) {
    return in.find("\r\n\r\n") != std::string::npos;
}

int handle_client(int fd, short revents, const ServerFlat& s, ConnState& st) {
    if (revents & (POLLHUP | POLLERR | POLLNVAL)) return ACT_CLOSE;

    int want = 0;

    if (!st.resp_ready && (revents & POLLIN)) {
        int rr = try_recv_all_ready(fd, st.in);
        if (rr < 0) return ACT_CLOSE;
        
        st.last_activity = time(NULL);  

        if (!headers_complete(st.in)) {
            want |= ACT_READ;
        } else {
            Request req; 
            Response resp;
            if (!req.parse(st.in)) {
                resp.setStatus(400);
                resp.setErrorBody(400);
            } else {
                Router router(s);
                resp = router.route(req);
            }
            st.out = resp.build();
            st.off = 0;
            st.resp_ready = true;
			// req.debugPrint();
        }
    }

    if (st.resp_ready && (revents & (POLLOUT | POLLIN))) {
        int wr = try_send_progress(fd, st.out, st.off);
        if (wr < 0) return ACT_CLOSE;
        
        st.last_activity = time(NULL);
        
        if (wr == 0) {
            want |= ACT_WRITE;
        } else {
            return ACT_CLOSE;
        }
    }

    if (want == 0) {
        want = st.resp_ready ? ACT_WRITE : ACT_READ;
    }
    return want;
}

