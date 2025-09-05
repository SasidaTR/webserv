#ifndef ROUTER_HPP
#define ROUTER_HPP

#include <string>
#include <fstream>
#include <sstream>

#include "Request.hpp"
#include "Response.hpp"
#include "../configuration/configParse.hpp"

class Router {
	private:
		const ServerFlat& server;
		std::string getContentType(const std::string& path) const;
		bool readFile(const std::string& path, std::string& content) const;

	public:
		Router(const ServerFlat& s);
		Response route(const Request& req) const;
};

#endif
