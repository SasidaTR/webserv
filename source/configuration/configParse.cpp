#include <fstream>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include "../include/configuration/configParse.hpp"

bool parse_location_block_from_line(std::istream &in, const std::string &header, ServerFlat &srv);

static std::string trimws(const std::string &s) {
    const char *ws = " \t\r\n";
    std::string::size_type a = s.find_first_not_of(ws);
    if (a == std::string::npos) return "";
    std::string::size_type b = s.find_last_not_of(ws);
    return s.substr(a, b - a + 1);
}

static std::string strip_trailing_semicolon(const std::string &s) {
    if (!s.empty() && s[s.size()-1] == ';') return s.substr(0, s.size()-1);
    return s;
}

static std::vector<std::string> split_words(const std::string &line) {
    std::vector<std::string> out;
    std::string cur;
    for (std::string::size_type i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (c==' ' || c=='\t' || c=='\r' || c=='\n') {
            if (!cur.empty()) { out.push_back(cur); cur.clear(); }
        } else {
            cur.push_back(c);
        }
    }
    if (!cur.empty()) out.push_back(cur);
    return out;
}


configParse::configParse(const std::string &path) {
    std::ifstream in(path.c_str());
    if (!in) throw std::runtime_error("config: cannot open file: " + path);

    std::string line;
    while (std::getline(in, line)) {
        std::string s = trimws(line);
        if (s.empty() || s[0] == '#') continue;

        if (s == "server {") {
            ServerFlat srv;
            while (std::getline(in, line)) {
                std::string t = trimws(line);
                if (t.empty() || t[0] == '#') continue;
                if (t[0] == '}') {
                    if (srv.root.empty()) throw std::runtime_error("config: server missing 'root'");
                    if (srv.index.empty()) srv.index = "index.html";
                    if (srv.host.empty()) srv.host = "127.0.0.1";
                    if (srv.port.empty()) srv.port = "8080";
                    _servers.push_back(srv);
                    break;
                }

                if (t.size() >= 9 && t.compare(0, 9, "location ") == 0) {
                    if (!parse_location_block_from_line(in, t, srv))
                        throw std::runtime_error("config: failed to parse location block");
                    continue;
                }

                t = strip_trailing_semicolon(t);
                std::vector<std::string> w = split_words(t);
                if (w.empty()) continue;

                const std::string key = w[0];

                if (key == "server_name") {
                    if (w.size() < 2) throw std::runtime_error("config: server_name expects a value");
                    std::string val;
                    for (std::vector<std::string>::size_type i = 1; i < w.size(); ++i) {
                        if (i > 1) val += " ";
                        val += w[i];
                    }
                    srv.name = val;
                    continue;
                }
                if (key == "host") {
                    if (w.size() != 2) throw std::runtime_error("config: host expects one value");
                    srv.host = w[1];
                    continue;
                }
                if (key == "port") {
                    if (w.size() != 2) throw std::runtime_error("config: port expects one value");
                    srv.port = w[1];
                    continue;
                }
                if (key == "root") {
                    if (w.size() < 2) throw std::runtime_error("config: root expects one value");
                    std::string val;
                    for (std::vector<std::string>::size_type i = 1; i < w.size(); ++i) {
                        if (i > 1) val += " ";
                        val += w[i];
                    }
                    srv.root = val;
                    continue;
                }
                if (key == "index") {
                    if (w.size() != 2) throw std::runtime_error("config: index expects one value");
                    srv.index = w[1];
                    continue;
                }

                throw std::runtime_error(std::string("config: unknown server directive: ") + key);
            }
            continue;
        }
    }

    if (_servers.empty())
        throw std::runtime_error("config: no server blocks found");
}

configParse::~configParse() {}

