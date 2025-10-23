#pragma once

#include "../webserv.hpp"

struct Location {
    std::string path;
    std::string root;
    std::string index;
    std::string alias;
    bool autoindex;
    std::vector<std::string> loc_methods;
    std::string client_max_body_size;
    //cgi : 
    std::vector<std::string> cgi_ext;
    std::vector<std::string> cgi_path;
    // redirect :
    std::string redirect_url;
    int redirect_code;
    // upload :
    std::string upload_dir;

    Location() : autoindex(false), redirect_code(0) {}
};

struct ServerFlat {
    std::string name;
    std::string host;
    std::string port;
    std::string root;
    std::string alias;
    std::string index;
    std::string client_max_body_size;
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