#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>

class Request {
	private:
		std::string method;
		std::string target;
		std::string version;

	public:
		bool parse_start_line(const std::string& req);
		std::string getMethod() const { return method; }
		std::string getTarget() const { return target; }
		std::string getVersion() const { return version; }
};

#endif
