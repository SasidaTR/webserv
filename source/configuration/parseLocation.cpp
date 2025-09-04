#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <cctype>
#include <iostream>
#include "../include/configuration/configParse.hpp"

namespace {

    static std::string ltrim(const std::string &s) {
        std::string::size_type i = 0;
        while (i < s.size()) {
            char c = s[i];
            if (c==' ' || c=='\t' || c=='\r' || c=='\n') ++i;
            else break;
        }
        return s.substr(i);
    }

    static std::string rtrim(const std::string &s) {
        if (s.empty()) return s;
        std::string::size_type i = s.size();
        while (i > 0) {
            char c = s[i-1];
            if (c==' ' || c=='\t' || c=='\r' || c=='\n') --i;
            else break;
        }
        return s.substr(0, i);
    }

    static std::string trim(const std::string &s) { return rtrim(ltrim(s)); }

    static std::string strip_comment(const std::string &s) {
        std::string::size_type h = s.find('#');
        if (h != std::string::npos) return s.substr(0, h);
        return s;
    }

    static bool starts_with_kw(const std::string &line, const std::string &kw) {
        if (line.size() < kw.size()) return false;
        return line.compare(0, kw.size(), kw) == 0;
    }

    static void split_words(const std::string &line, std::vector<std::string> &out) {
        out.clear();
        std::string cur;
        for (std::string::size_type i = 0; i < line.size(); ++i) {
            char c = line[i];
            if (c==' ' || c=='\t' || c=='\r' || c=='\n' || c==';') {
                if (!cur.empty()) { out.push_back(cur); cur.clear(); }
            } else {
                cur.push_back(c);
            }
        }
        if (!cur.empty()) out.push_back(cur);
    }

    static bool next_useful_line(std::istream &in, std::string &outLine) {
        outLine.clear();
        std::string raw;
        while (std::getline(in, raw)) {
            if (!raw.empty() && raw[raw.size()-1] == '\r') raw.erase(raw.size()-1);
            raw = strip_comment(raw);
            raw = trim(raw);
            if (!raw.empty()) { outLine = raw; return true; }
        }
        return false;
    }

}

bool parse_location_block_from_line(std::istream &in, const std::string &header, ServerFlat &cur) {
    std::string line = strip_comment(header);
    line = trim(line);
    if (!starts_with_kw(line, "location")) return false;

    std::vector<std::string> toks;
    split_words(line, toks);
    if (toks.size() < 2) throw std::runtime_error("location: missing path");

    Location L;
    L.path = toks[1];
    L.autoindex = false;

    bool have_open_brace = (!toks.empty() && toks[toks.size()-1] == "{");
    if (!have_open_brace) {
        std::string maybe;
        if (!next_useful_line(in, maybe))
            throw std::runtime_error("location: unexpected EOF after header");
        if (maybe != "{")
            throw std::runtime_error("location: expected '{' after header");
    }

    std::string body;
    while (next_useful_line(in, body)) {
        if (body == "}") {
            cur.locations.push_back(L);
            return true;
        }
        std::vector<std::string> w;
        split_words(body, w);
        if (w.empty()) continue;

        const std::string key = w[0];

        if (key == "root") {
            if (w.size() < 2) throw std::runtime_error("location: root requires a path");
            L.root = w[1];
            continue;
        }
        if (key == "index") {
            if (w.size() < 2) throw std::runtime_error("location: index requires a file");
            L.index = w[1];
            continue;
        }
        if (key == "autoindex") {
            if (w.size() < 2) throw std::runtime_error("location: autoindex requires 'on' or 'off'");
            L.autoindex = (w[1] == "on");
            continue;
        }
        if (key == "cgi_path") {
            if (w.size() < 2) throw std::runtime_error("location: cgi_path requires at least one interpreter");
            for (std::vector<std::string>::size_type i = 1; i < w.size(); ++i) L.cgi_path.push_back(w[i]);
            continue;
        }
        if (key == "cgi_ext") {
            if (w.size() < 2) throw std::runtime_error("location: cgi_ext requires at least one extension");
            for (std::vector<std::string>::size_type i = 1; i < w.size(); ++i) L.cgi_ext.push_back(w[i]);
            continue;
        }

        throw std::runtime_error(std::string("location: unknown directive: ") + key);
    }

    throw std::runtime_error("location: missing closing '}'");
}
