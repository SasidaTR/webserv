#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <string>
# include <map>

class Request {
	private:
		std::string method;
		std::string path;
		std::string version;
		std::map<std::string, std::string> headers;
		std::string body;
	public:
		Request();
		~Request();
		void parse(const std::string &raw);
};

#endif
