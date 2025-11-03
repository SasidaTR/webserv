/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jvittoz <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/02 19:49:00 by jvittoz           #+#    #+#             */
/*   Updated: 2025/11/02 19:49:04 by jvittoz          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/http/HttpServer.hpp"
#include "../../include/http/Request.hpp"
#include "../../include/http/Response.hpp"
#include "../../include/http/Router.hpp"
#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <poll.h>
#include <algorithm>
#include <cctype>
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
            continue;
        }
        if (n == 0) {
            return -1; 
        }
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 0;
        }
        return -1;
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

static void addEnvVar(std::vector<std::string> &env, const std::string &key, const std::string &value) {
    env.push_back(key + "=" + value);
}

static std::string intToString(size_t n) {
    std::ostringstream oss;
    oss << n;
    return oss.str();
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

    size_t header_end = st.in.find("\r\n\r\n");
    if (header_end != std::string::npos) {
        size_t body_start = header_end + 4;
        if (st.in.size() > body_start) {
            size_t avail = st.in.size() - body_start;
            size_t take  = std::min(avail, st.body_expected);
            st.body_buf.assign(st.in, body_start, take);
            st.body_received = take;
            if (st.body_received >= st.body_expected)
                st.body_done = true;
        }
    }

    st.env.clear();
    addEnvVar(st.env, "GATEWAY_INTERFACE", "CGI/1.1");
    addEnvVar(st.env, "REQUEST_METHOD", req.getMethod());
    addEnvVar(st.env, "SERVER_PROTOCOL", req.getVersion());
    addEnvVar(st.env, "SCRIPT_NAME", req.getTargetPath());
    addEnvVar(st.env, "SCRIPT_FILENAME", st.cgi_script);
    addEnvVar(st.env, "QUERY_STRING", req.getQueryString());
    addEnvVar(st.env, "PATH_INFO", req.getPathInfo());
    addEnvVar(st.env, "PATH_TRANSLATED", st.cgi_script);

    if (req.contentLength() > 0)
        addEnvVar(st.env, "CONTENT_LENGTH", intToString(req.contentLength()));
    if (!req.getHeader("content-type").empty())
        addEnvVar(st.env, "CONTENT_TYPE", req.getHeader("content-type"));

    if (req.hasExpect100()) {
        st.out = "HTTP/1.1 100 Continue\r\n\r\n";
        st.off = 0;
        st.resp_ready = true;
        st.expect_continue = true;
        st.reading_body = true;
        st.in.clear();
        return ACT_WRITE | ACT_READ;
    }

    return ACT_READ;
}


static const ServerFlat& select_virtual_host(const std::vector<size_t>& candidates,
                                             const std::vector<ServerFlat>& servers,
                                             const Request& req,
                                             const ServerFlat& fallback)
{
    std::string host = req.getHeader("host");
    if (!host.empty()) {
        size_t colon = host.find(':');
        if (colon != std::string::npos)
            host = host.substr(0, colon);
        std::transform(host.begin(), host.end(), host.begin(), ::tolower);
    }

    for (size_t i = 0; i < candidates.size(); ++i) {
        const ServerFlat& srv = servers[candidates[i]];
        if (srv.name == host)
            return srv;
    }
    return fallback;
}


int handle_client(int fd, short revents, const ServerFlat& s, ConnState& st) {
    if (revents & (POLLHUP | POLLERR | POLLNVAL)) return ACT_CLOSE;

    int want = 0;

    if (!st.resp_ready && (revents & POLLIN)) {
        int rr = try_recv_all_ready(fd, st.in);
        if (rr < 0) return ACT_CLOSE;
        st.last_activity = time(NULL);

        if (st.reading_body) {
            const size_t just_added = st.in.size();
            st.body_buf.append(st.in);
            st.body_received += just_added;
            st.in.clear();

            if (st.body_expected && st.body_received >= st.body_expected) {
                st.body_done = true;
                st.reading_body = false;
            }
            want |= ACT_READ;
            std::cerr << "[BODY] received " << just_added
                      << " bytes (total " << st.body_received
                      << " / " << st.body_expected << ")\n";
            return want;
        }

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
                const ServerFlat& chosen =
                    (st.vhost_candidates && st.servers_all)
                    ? select_virtual_host(*st.vhost_candidates, *st.servers_all, req, s)
                    : s;

                Router router(chosen);
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

    if (st.resp_ready && (revents & POLLOUT)) {
        int wr = try_send_progress(fd, st.out, st.off);
        if (wr < 0) return ACT_CLOSE;

        st.last_activity = time(NULL);

        if (wr == 0) {
            want |= ACT_WRITE;
        } else {
            st.resp_ready = false;

            if (st.expect_continue) {
                st.expect_continue = false;
                want |= ACT_READ;
            }
            else if (st.phase == CGI_SPAWN || st.phase == CGI_STREAM) {
                want |= ACT_READ;
            }
            else {
                return ACT_CLOSE;
            }
        }
    }

    if (want == 0) {
        if (st.reading_body && !st.body_done)
            want = ACT_READ;
        else if (st.phase == CGI_SPAWN || st.phase == CGI_STREAM)
            want = ACT_READ;
        else
            want = (st.resp_ready ? ACT_WRITE : ACT_READ);
    }

    return want;
}
