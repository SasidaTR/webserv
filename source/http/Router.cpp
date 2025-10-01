#include "../../include/http/Router.hpp"

Router::Router(const ServerFlat& s) : server(s) {}

std::string Router::getContentType(const std::string& path) const {
	if (path.size() >= 5 && path.substr(path.size() - 5) == ".html") return "text/html";
	if (path.size() >= 4 && path.substr(path.size() - 4) == ".css") return "text/css";
	if (path.size() >= 3 && path.substr(path.size() - 3) == ".js") return "application/javascript";
	if (path.size() >= 4 && path.substr(path.size() - 4) == ".png") return "image/png";
	if (path.size() >= 4 && path.substr(path.size() - 4) == ".jpg") return "image/jpeg";
	return "text/plain";
}

bool Router::readFile(const std::string& path, std::string& content) const {
	std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
	if (!file) return false;
	std::ostringstream ss;
	ss << file.rdbuf();
	content = ss.str();
	return true;
}

static bool isCGI(const Request &req)
{
    std::string temp = req.getTarget();

    //prefix
    int i = 0;
    std::string prefix = "/cgi-bin/";
    while (temp[i] && prefix[i]) {
        if (temp[i] != prefix[i])
            break;
        ++i;
    }
    bool hasPrefix = (prefix[i] == '\0');

    //postfix
    std::string postfix = ".sh";
    int len = 0;
    while (postfix[len]) 
		++len;
    int slen = temp.size();
    bool hasPostfix = false;
    if (slen >= len) {
        hasPostfix = true;
        for (int j = 0; j < len; ++j) {
            if (temp[slen - len + j] != postfix[j]) {
                hasPostfix = false;
                break;
            }
        }
    }

    return hasPrefix || hasPostfix;
}
/*
Response Router::handleCGI(){

}
*/

Response Router::route(const Request& req) const {
	Response resp;

	if (isCGI(req))
	{
		//resp = handleCGI(resp, req);
		return resp;
	}

	if (req.getMethod() != "GET") {
		resp.setStatus(405);
		resp.setErrorBody(405);
		return resp;
	}

	std::string path;
	if (req.getTarget() == "/")
		path = server.root + "/" + server.index;
	else
		path = server.root + req.getTarget();

	std::string body;
	if (readFile(path, body)) {
		resp.setStatus(200);
		resp.setContentType(getContentType(path));
		resp.setBody(body);
	} else {
		resp.setStatus(404);
		resp.setErrorBody(404);
	}

	return resp;
}
