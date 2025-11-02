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

#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <cctype>
#include <iostream>
#include "../include/configuration/configParse.hpp"
#include "../include/webserv.hpp"

static std::string trimws(const std::string &s) {
	const char *ws = " \t\r\n";
	std::string::size_type a = s.find_first_not_of(ws);
	if (a == std::string::npos) return "";
	std::string::size_type b = s.find_last_not_of(ws);
	return s.substr(a, b - a + 1);
}

static bool starts_with(const std::string &s, const std::string &p) {
	return s.size() >= p.size() && s.compare(0, p.size(), p) == 0;
}

static std::vector<std::string> split_words(const std::string &line) {
	std::vector<std::string> out;
	std::string cur;
	for (std::string::size_type i = 0; i < line.size(); ++i) {
		char c = line[i];
		if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
			if (!cur.empty()) { out.push_back(cur); cur.clear(); }
		} else {
			out.push_back(std::string(1, c));
			while (i + 1 < line.size()) {
				char d = line[i+1];
				if (d == ' ' || d == '\t' || d == '\r' || d == '\n') break;
				cur.push_back(d);
				++i;
			}
			if (!cur.empty()) { out.back() += cur; cur.clear(); }
		}
	}
	if (!cur.empty()) out.push_back(cur);
	return out;
}

static std::string join_from(const std::vector<std::string> &w, std::vector<std::string>::size_type from) {
	std::string res;
	for (std::vector<std::string>::size_type i = from; i < w.size(); ++i) {
		if (!res.empty()) res += " ";
		res += w[i];
	}
	return res;
}

static std::string read_directive_line_with_semicolon(int fd, const std::string &firstLine) {
	std::string s = trimws(firstLine);
	if (!s.empty() && s[s.size()-1] == ';') return s.substr(0, s.size()-1);
	std::string line;
	while (getline_fd(fd, line)) {
		std::string t = trimws(line);
		if (!t.empty()) {
			s += " ";
			s += t;
			if (!t.empty() && t[t.size()-1] == ';') {
				s.erase(s.size() - 1);
				break;
			}
		}
	}
	return s;
}

bool parse_location_block_from_line(int fd, const std::string &header, ServerFlat &srv) {
	std::string s = trimws(header);
	if (!starts_with(s, "location "))
		throw std::runtime_error("location: header must start with 'location'");

	s = trimws(s.substr(9));
	std::string path;
	std::string::size_type brace = s.find('{');
	if (brace != std::string::npos) {
		path = trimws(s.substr(0, brace));
	} else {
		path = trimws(s);
		std::string line;
		bool opened = false;
		while (getline_fd(fd, line)) {
			if (line.find('{') != std::string::npos) { opened = true; break; }
			if (trimws(line).empty()) continue;
		}
		if (!opened) throw std::runtime_error("location: missing '{' after header");
	}
	if (path.empty() || path[0] != '/')
		throw std::runtime_error("location: path must start with '/'");

	Location loc;
	loc.path = path;
	loc.root = srv.root;
	loc.index = srv.index;
	loc.autoindex = false;

	std::string line;
	while (getline_fd(fd, line)) {
		std::string t = trimws(line);
		if (t.empty() || t[0] == '#') continue;
		if (t[0] == '}') {
			srv.locations.push_back(loc);
			return true;
		}

		std::string d = read_directive_line_with_semicolon(fd, t);
		if (d.empty()) continue;

		std::vector<std::string> w = split_words(d);
		if (w.empty()) continue;

		const std::string key = w[0];

		if (key == "root") { loc.root = join_from(w, 1); continue; }
		if (key == "index") { loc.index = w[1]; continue; }
		if (key == "autoindex") { loc.autoindex = (w[1] == "on"); continue; }
		if (key == "methods") {
			loc.loc_methods.clear();
			for (std::vector<std::string>::size_type i = 1; i < w.size(); ++i)
				loc.loc_methods.push_back(w[i]);
			continue;
		}
		if (key == "cgi_path") { loc.cgi_path.clear(); for (size_t i = 1; i < w.size(); ++i) loc.cgi_path.push_back(w[i]); continue; }
		if (key == "cgi_ext") { loc.cgi_ext.clear(); for (size_t i = 1; i < w.size(); ++i) loc.cgi_ext.push_back(w[i]); continue; }
		if (key == "client_max_body_size") { loc.client_max_body_size = w[1]; continue; }
		if (key == "return" || key == "redirect") {
			loc.redirect_code = (w.size() == 2 ? 301 : atoi(w[1].c_str()));
			loc.redirect_url = (w.size() == 2 ? w[1] : w[2]);
			continue;
		}
		if (key == "upload_dir") { loc.upload_dir = w[1]; continue; }
		if (key == "alias") { loc.alias = w[1]; continue; }

		throw std::runtime_error(std::string("location: unknown directive: ") + key);
	}

	throw std::runtime_error("location: missing closing '}'");
}
