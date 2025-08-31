#include <fstream>
#include <iostream>
#include "../include/hpp/configParse.hpp"

std::string configParse::trim(const std::string& s) {
    const std::string ws = " \t\r\n";
    size_t start = s.find_first_not_of(ws);
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(ws);
    return s.substr(start, end - start + 1);
}

bool configParse::starts_with(const std::string& s, const std::string& prefix) {
    if (s.size() < prefix.size()) return false;
    return s.compare(0, prefix.size(), prefix) == 0;
}

std::string configParse::setValue(const std::string& line) {
    size_t sep = line.find(':');
    if (sep == std::string::npos)
        throw std::runtime_error("config: missing ':' in line: " + line);
    std::string value = line.substr(sep + 1);
    size_t hash = value.find('#');
    if (hash != std::string::npos)
        value = value.substr(0, hash);

    return trim(value);
}

bool configParse::down_the_list(const std::string& line, ServerFlat &s) {
    if (starts_with(line, "port:")) {
        s.port = setValue(line);
    } else if (starts_with(line, "root:")) {
        s.root = setValue(line);
    } else if (starts_with(line, "index:")) {
        s.index = setValue(line);
    } else if (starts_with(line, "location:")) {
        s.location = setValue(line);
    } else if (starts_with(line, "host:")) {
        s.host = setValue(line);
    } else {
        return false;
    }
    return true;
}

configParse::configParse(const std::string& file) : _filename(file) {
    std::ifstream in(file.c_str());
    if (!in.is_open())
        throw std::runtime_error("Could not open config file: " + file);

    std::string raw;
    bool haveActive = false;

    while (std::getline(in, raw)) {
        if (!raw.empty() && raw[raw.size()-1] == '\r')
            raw.erase(raw.size()-1);
        size_t hash = raw.find('#');
        if (hash != std::string::npos) raw = raw.substr(0, hash);
        std::string line = trim(raw);
        if (line.empty()) continue;

        if (starts_with(line, "server:")) {
            _servers.push_back(ServerFlat());
            haveActive = true;
            continue;
        }

        if (!haveActive) {
            throw std::runtime_error("config: directive before 'server:' marker: " + line);
        }

        ServerFlat &cur = _servers.back();
        if (!down_the_list(line, cur)) {
            throw std::runtime_error("config: unknown directive: " + line);
        }
    }

    in.close();
    if (_servers.empty())
        throw std::runtime_error("config: no server defined");
    for (size_t i = 0; i < _servers.size(); ++i) {
        if (_servers[i].port.empty())
            throw std::runtime_error("config: server #" + std::string(1, '0' + (char)i) + " missing 'port:'");
        if (_servers[i].root.empty())
            throw std::runtime_error("config: server #" + std::string(1, '0' + (char)i) + " missing 'root:'");
    }
}

configParse::~configParse() {}
