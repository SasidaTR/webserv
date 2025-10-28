#ifndef CGI_UTILS_HPP
#define CGI_UTILS_HPP

#include <string>
#include <vector>

struct Location;

class CGIUtils {
public:
	static std::string getFileExtension(const std::string& filename);
	static bool isScriptFile(const std::string& path, const Location& loc);
	static std::string findInterpreter(const std::string& scriptPath, const Location& loc);
	static std::string cleanUrl(const std::string& url);
};

#endif