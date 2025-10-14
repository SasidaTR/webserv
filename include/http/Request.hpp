#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <map>

class Request {
	private:
		std::string method;
		std::string target;
		std::string version;
		std::map<std::string, std::string> headers;
		std::string body;
		std::string dechunkBody(const std::string& chunked);

	public:
		bool parse(const std::string& raw);
		std::string getMethod() const { return method; }
		std::string getTarget() const { return target; }
		std::string getVersion() const { return version; }
		std::string getHeader(const std::string& key) const;
		const std::map<std::string, std::string>& getHeaders() const { return headers; }
		std::string getBody() const { return body; }
		bool isValidMethod() const;
		void debugPrint() const;
};

#endif
