#ifndef ROUTER_HPP
#define ROUTER_HPP

#include <string>

#include "Request.hpp"
#include "Response.hpp"
#include "../configuration/configParse.hpp"
#include "../cgi/CGIHandler.hpp"

class Router {
	private:
		const ServerFlat& server;
		CGIHandler cgiHandler;
		
		const Location* findMatchingLocation(const std::string& path) const;
		bool isMethodAllowed(const Request& req, const Location* loc) const;
		std::string resolvePath(const Request& req, const Location* loc) const;
		bool checkBodySize(const Request& req, const Location* loc) const;

	public:
		Router(const ServerFlat& s);
		Response route(const Request& req) const;
};

#endif
