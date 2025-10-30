#ifndef CGI_HANDLER_HPP
#define CGI_HANDLER_HPP

#include <string>
#include <vector>

class Request;
class Response;
struct Location;

class CGIHandler {
public:
	CGIHandler();
	~CGIHandler();

	bool canHandle(const Request& req, const Location& loc, const std::string& resolvedPath) const;

private:
	static std::string extNoDotLower(std::string p);
	static bool hasExt(const std::vector<std::string>& exts, const std::string& e);
	static std::string pickRunner(const Location& loc, const std::string& ext);
};

#endif
