#ifndef DIRECTORY_HANDLER_HPP
#define DIRECTORY_HANDLER_HPP

#include <string>

class DirectoryHandler {
public:
	static bool isDirectory(const std::string& path);
	static std::string generateListing(const std::string& dirPath, const std::string& urlPath);
	
private:
	static std::string generateHTMLHeader(const std::string& urlPath);
	static std::string generateHTMLFooter();
	static std::string generateParentLink(const std::string& urlPath);
};

#endif
