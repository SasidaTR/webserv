#include "../../include/http/HttpServer.hpp"
#include "../../include/http/Request.hpp"
#include "../../include/http/Response.hpp"
#include "../../include/http/Router.hpp"
#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <cerrno>
#include <poll.h>


// usfule status code
struct ServerFlat;

struct ConnState;

//send rec functions
static int try_recv_all_ready(int fd, std::string &buf) {
    char tmp[10000];
    for (;;) {
        ssize_t n = recv(fd, tmp, sizeof(tmp), 0);
        if (n > 0) { buf.append(tmp, n); continue; }
        if (n == 0) return -1; 
        if (errno == EAGAIN || errno == EWOULDBLOCK) return 0;
        if (errno == EINTR) continue;
        return -1;
    }
}

static int try_send_progress(int fd, const std::string &out, size_t &off) {
    while (off < out.size()) {
        ssize_t n = send(fd, out.data() + off, out.size() - off, 0);
        if (n > 0) { off += (size_t)n; continue; }
        if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) return 0;
        if (errno == EINTR) continue;
        return -1;
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

        if (!headers_complete(st.in)) {
            want |= ACT_READ;
        } else {
            Request req; Response resp;
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
			req.debugPrint();
        }
    }

    if (st.resp_ready && (revents & (POLLOUT | POLLIN))) {
        int wr = try_send_progress(fd, st.out, st.off);
        if (wr < 0) return ACT_CLOSE;
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

