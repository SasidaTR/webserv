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

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include "../../include/webserv.hpp"

#define LOGCGI(fmt, ...) do { \
	fprintf(stderr, "[CGI] " fmt "\n", ##__VA_ARGS__); \
} while (0)

static inline void set_nonblock_fd(int fd) {
	int fl = fcntl(fd, F_GETFL, 0);
	if (fl != -1) fcntl(fd, F_SETFL, fl | O_NONBLOCK);
}

void log_env_connstate(const ConnState &st) {
	LOGCGI("=== ConnState environment (%zu entries) ===", st.env.size());
	for (size_t i = 0; i < st.env.size(); ++i)
		LOGCGI("env %s", st.env[i].c_str());
	LOGCGI("=== End of ConnState environment ===");
}

void spawn_cgi(ConnState& st) {
    int pin[2], pout[2];
    if (pipe(pin) == -1 || pipe(pout) == -1)
        throw std::runtime_error("pipe failed");

    pid_t pid = fork();
    if (pid < 0)
        throw std::runtime_error("fork failed");

    if (pid == 0) {
        // --- child ---
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
        for (size_t i = 0; i < st.env.size(); ++i)
            envp.push_back(const_cast<char*>(st.env[i].c_str()));
        envp.push_back(NULL);

        execve(argv[0], &argv[0], &envp[0]);

        const char* msg =
            "Status: 500 Internal Server Error\r\n"
            "Content-Type: text/plain\r\n\r\nexecve failed\n";
        (void)!write(1, msg, std::strlen(msg));
        _exit(127);
    }

    st.cgi_pid = pid;

    st.cgi_in  = pin[1];  close(pin[0]);
    st.cgi_out = pout[0]; close(pout[1]);

    set_nonblock_fd(st.cgi_in);
    set_nonblock_fd(st.cgi_out);

    st.cgi_in_open  = true;
    st.cgi_out_open = true;
    st.cgi_start_time = time(NULL);
    st.is_cgi_running = true;

    st.cgi_written = 0;
}


