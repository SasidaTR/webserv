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
	char tmp[8192];
	for (;;) {
		ssize_t n = recv(fd, tmp, sizeof(tmp), 0);
		if (n > 0) { 
			buf.append(tmp, n); 
			if (buf.size() > 10485760) {
				return -1;
			}
			continue; 
		}
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

static int prepare_cgi(const Request& req, const Response& resp, ConnState& st) {
    st.phase           = CGI_SPAWN;
    st.cgi_script      = resp.cgiScript();
    st.cgi_interpreter = resp.cgiInterpreter();

    st.body_expected = req.isChunked() ? 0 : req.contentLength();
    st.body_received = 0;
    st.chunked       = req.isChunked();
    st.body_done     = (!st.chunked && st.body_expected == 0);

    st.body_buf.clear();
    st.cgi_written = 0;

    if (req.hasExpect100()) {
        st.out = "HTTP/1.1 100 Continue\r\n\r\n";
        st.off = 0;
        st.resp_ready = true;
        return ACT_WRITE | ACT_READ;
    }
    return ACT_READ;
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
            Request  req;
            Response resp;

            if (!req.parse(st.in)) {
                resp.setStatus(400);
                resp.setErrorBody(400);
                st.out = resp.build();
                st.off = 0;
                st.resp_ready = true;
                want |= ACT_WRITE;
            } else {
                Router router(s);
                resp = router.route(req);

                if (resp.isCgi()) {
                    want |= prepare_cgi(req, resp, st);
                } else {
                    st.out = resp.build();
                    st.off = 0;
                    st.resp_ready = true;
                    want |= ACT_WRITE;
                    req.debugPrint();
                }
            }
        }
    }

    if (st.resp_ready && (revents & (POLLOUT | POLLIN))) {
        int wr = try_send_progress(fd, st.out, st.off);
        if (wr < 0) return ACT_CLOSE;

        st.last_activity = time(NULL);

        if (wr == 0) {
            want |= ACT_WRITE;
        } else {
            st.resp_ready = false;

            if (st.phase == CGI_SPAWN || st.phase == CGI_STREAM)
                want |= ACT_READ;
            else
                return ACT_CLOSE;
        }
    }

    if (want == 0)
        want = (st.phase == CGI_SPAWN || st.phase == CGI_STREAM) ? ACT_READ
                 : (st.resp_ready ? ACT_WRITE : ACT_READ);
    return want;
}


