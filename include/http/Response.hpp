#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <sstream>
#include <map>

class Response {
	private:
		int statusCode;
		std::string reasonPhrase;
		std::string contentType;
		std::string body;
		std::string redirectLocation;
		static std::map<int, std::string> initReasonPhrases();
		static const std::map<int, std::string> reasonPhrases;

	public:
		Response();
		void setStatus(int code);
		void setContentType(const std::string& ctype);
		void setBody(const std::string& b);
		void setErrorBody(int code, const std::string& errorDir = "html/error");
		void setRedirect(const std::string& location);
		std::string build() const;
};

#endif
