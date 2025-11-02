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

#include "../include/webserv.hpp"

bool getline_fd(int fd, std::string& out) {
	static std::map<int, std::string> stash;
	std::string& buf = stash[fd];

	for (;;) {
		size_t pos = std::string::npos;
		for (size_t i = 0; i < buf.size(); ++i) {
			if (buf[i] == '\n') { pos = i; break; }
		}

		if (pos != std::string::npos) {
			out.assign(buf, 0, pos);
			buf.erase(0, pos + 1);
			return true;
		}

		char tmp[4096];
		ssize_t n = ::read(fd, tmp, sizeof(tmp));
		if (n > 0) {
			buf.append(tmp, tmp + n);
			continue;
		}

		if (n == 0) {
			if (!buf.empty()) {
				out.swap(buf);
				buf.clear();
				return true;
			}
			stash.erase(fd);
			return false;
		}

		stash.erase(fd);
		return false;
	}
}
