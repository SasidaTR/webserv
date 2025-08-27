#pragma once

class configParse {
    private :
        std::string port;
        std::string root;
        std::string index;

    public :
        configParse(std::string file);
        ~configParse();
}