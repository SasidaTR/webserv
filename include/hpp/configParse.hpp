#pragma once

#include "./webserv.hpp"

struct ServerFlat {
    std::string host;
    std::string port;
    std::string root;
    std::string index;
    std::string location;
};

class configParse {
private:
    std::string                 _filename;
    std::vector<ServerFlat>     _servers;

    static std::string trim(const std::string& s);
    static bool starts_with(const std::string& s, const std::string& prefix);
    static std::string setValue(const std::string& line);
    static bool down_the_list(const std::string& line, ServerFlat &s);

public:
    configParse(const std::string& file);
    ~configParse();

    std::vector<ServerFlat>& getServers() { return _servers; }
};
