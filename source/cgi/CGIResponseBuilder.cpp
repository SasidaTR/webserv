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

#include <sstream>
#include <string>
#include <vector>
#include <utility>
#include <iostream>
#include "../../include/webserv.hpp"

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

void build_http_from_cgi(ConnState& st) {
	const std::string& raw = st.cgi_raw;
	std::cout << "[CGI] raw output (first 200 chars):\n" << raw.substr(0, 200) << std::endl;

	size_t sep = raw.find("\r\n\r\n");
	if (sep == std::string::npos) sep = raw.find("\n\n");
	if (sep == std::string::npos) {
		const std::string body = "Bad Gateway: CGI produced no headers\n";
		std::ostringstream os;
		os << "HTTP/1.1 502 Bad Gateway\r\nContent-Type: text/plain\r\nContent-Length: " << body.size() << "\r\nConnection: close\r\n\r\n" << body;
		st.out = os.str();
		return;
	}

	std::string head = raw.substr(0, sep);
	std::string body = raw.substr(sep + ((raw[sep] == '\r') ? 4 : 2));
	std::istringstream hs(head);
	std::string line;
	int status_code = 200;
	std::vector<std::pair<std::string, std::string> > hdrs;

	while (std::getline(hs, line)) {
		if (!line.empty() && line[line.size() - 1] == '\r') line = line.substr(0, line.size() - 1);
		if (line.empty()) continue;
		size_t colon = line.find(':');
		if (colon == std::string::npos) continue;

		std::string key = line.substr(0, colon);
		std::string val = line.substr(colon + 1);
		while (!val.empty() && (val[0] == ' ' || val[0] == '\t')) val = val.substr(1);

		if (key == "Status" || key == "Status:") {
			std::istringstream ss(val); int c = 0; ss >> c;
			if (c > 0) status_code = c;
		} else {
			hdrs.push_back(std::make_pair(key, val));
		}
	}

	bool has_cl = false, has_conn = false;
	for (size_t i = 0; i < hdrs.size(); ++i) {
		std::string k = hdrs[i].first;
		for (size_t j = 0; j < k.size(); ++j) if (k[j] >= 'A' && k[j] <= 'Z') k[j] = char(k[j] - 'A' + 'a');
		if (k == "content-length") has_cl = true;
		if (k == "connection") has_conn = true;
	}

	std::ostringstream os;
	os << "HTTP/1.1 " << status_code << " " << http_reason_from_code(status_code) << "\r\n";
	for (size_t i = 0; i < hdrs.size(); ++i) os << hdrs[i].first << ": " << hdrs[i].second << "\r\n";
	if (!has_cl) os << "Content-Length: " << body.size() << "\r\n";
	if (!has_conn) os << "Connection: close\r\n";
	os << "\r\n" << body;

	st.out = os.str();
}
