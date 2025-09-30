#pragma once

#include "../webserv.hpp"

struct Location {
    std::string path;
    std::string root;
    std::string index;
    bool autoindex;
    //cgi : 
    std::vector<std::string> cgi_ext;
    std::vector<std::string> cgi_path;
    std::vector<std::string> loc_methods;

    Location() : autoindex(false) {}
};

struct ServerFlat {
    std::string name;
    std::string host;
    std::string port;
    std::string root;
    std::string index;
    std::vector<std::string> ser_methods;
    std::vector<Location> locations;

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
