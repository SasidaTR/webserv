#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <sstream>

class Response {
	private:
		std::string status;
		std::string contentType;
		std::string body;

	public:
		Response();
		void setStatus(const std::string& s);
		void setContentType(const std::string& ctype);
		void setBody(const std::string& b);
		std::string build() const;
};

#endif
