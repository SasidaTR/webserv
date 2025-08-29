#include "../../include/http/Request.hpp"

bool Request::parse_start_line(const std::string& req) {
	size_t e = req.find("\r\n");
	if (e == std::string::npos) return false;
	std::string line = req.substr(0, e);

	size_t p1 = line.find(' ');
	if (p1 == std::string::npos) return false;
	size_t p2 = line.find(' ', p1 + 1);
	if (p2 == std::string::npos) return false;

	method = line.substr(0, p1);
	target = line.substr(p1 + 1, p2 - (p1 + 1));
	version = line.substr(p2 + 1);
	return true;
}
