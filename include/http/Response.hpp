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
		std::string cgi_script;
		std::string cgi_interpreter;
		bool is_cgi;

	public:
		Response();
		void setStatus(int code);
		void setContentType(const std::string& ctype);
		void setBody(const std::string& b);
		void setErrorBody(int code, const std::string& errorDir = "html/error");
		void setRedirect(const std::string& location);
		std::string build() const;
		void markAsCgi(const std::string& script, const std::string& interpreter) {
			is_cgi = true;
			cgi_script = script;
			cgi_interpreter = interpreter;
		}
		bool isCgi() const { return is_cgi; }
		const std::string& cgiScript() const { return cgi_script; }
		const std::string& cgiInterpreter() const { return cgi_interpreter; }
};

#endif
