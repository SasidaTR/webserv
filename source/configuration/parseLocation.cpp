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
                s.erase(s.size()-1);
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
    loc.path  = path;
    loc.root  = srv.root;
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

        if (key == "root") {
            if (w.size() < 2) throw std::runtime_error("location: 'root' expects one value");
            loc.root = join_from(w, 1);
            continue;
        }
        if (key == "index") {
            if (w.size() != 2) throw std::runtime_error("location: 'index' expects one value");
            loc.index = w[1];
            continue;
        }
        if (key == "autoindex") {
            if (w.size() != 2) throw std::runtime_error("location: 'autoindex' expects on/off");
            if (w[1] == "on") loc.autoindex = true;
            else if (w[1] == "off") loc.autoindex = false;
            else throw std::runtime_error("location: autoindex must be 'on' or 'off'");
            continue;
        }
        if (key == "methods") {
            if (w.size() < 2) throw std::runtime_error("location: 'allow_methods' needs at least one method");
            loc.loc_methods.clear();
            for (std::vector<std::string>::size_type i = 1; i < w.size(); ++i) {
                const std::string m = w[i];
                if (m != "GET" && m != "POST" && m != "DELETE" && m != "PUT" && m != "HEAD")
                    throw std::runtime_error(std::string("location: unsupported method '") + m + "'");
                loc.loc_methods.push_back(m);
            }
            continue;
        }
        if (key == "cgi_path") {
            if (w.size() < 2) throw std::runtime_error("location: 'cgi_path' needs at least one interpreter");
            loc.cgi_path.clear();
            for (std::vector<std::string>::size_type i = 1; i < w.size(); ++i)
                loc.cgi_path.push_back(w[i]);
            continue;
        }
        if (key == "cgi_ext") {
            if (w.size() < 2) throw std::runtime_error("location: 'cgi_ext' needs at least one extension");
            loc.cgi_ext.clear();
            for (std::vector<std::string>::size_type i = 1; i < w.size(); ++i)
                loc.cgi_ext.push_back(w[i]);
            continue;
        }
        loc.client_max_body_size="1m";
        if (key == "client_max_body_size") {
            if (w.size() != 2) throw std::runtime_error("config: expexts one argument");
            loc.client_max_body_size.clear();
            loc.client_max_body_size = w[1];
            continue;
        }
        if (key == "return" || key == "redirect") {
            if (w.size() < 2) throw std::runtime_error("location: 'return/redirect' needs at least url");
            if (w.size() == 2) {
                loc.redirect_code = 301;
                loc.redirect_url = w[1];
            } else {
                int code = atoi(w[1].c_str());
                if (code < 300 || code > 399) 
                    throw std::runtime_error("location: redirect code must be 3xx");
                loc.redirect_code = code;
                loc.redirect_url = w[2];
            }
            continue;
        }
        if (key == "upload_dir") {
            if (w.size() != 2) throw std::runtime_error("location: 'upload_dir' expects one path");
            loc.upload_dir = w[1];
            continue;
        }

        throw std::runtime_error(std::string("location: unknown directive: ") + key);
    }

    throw std::runtime_error("location: missing closing '}'");
}