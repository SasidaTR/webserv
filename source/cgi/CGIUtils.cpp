#include "../../include/cgi/CGIUtils.hpp"
#include "../../include/configuration/configParse.hpp"

std::string CGIUtils::getFileExtension(const std::string& filename) {
	size_t dotPos = filename.find_last_of('.');
	if (dotPos == std::string::npos || dotPos == filename.length() - 1)
		return "";
	return filename.substr(dotPos);
}

std::string CGIUtils::cleanUrl(const std::string& url) {
	size_t queryPos = url.find('?');
	if (queryPos != std::string::npos)
		return url.substr(0, queryPos);
	return url;
}

bool CGIUtils::isScriptFile(const std::string& path, const Location& loc) {
	if (loc.cgi_ext.empty())
		return false;
	std::string extension = getFileExtension(cleanUrl(path));
	for (size_t i = 0; i < loc.cgi_ext.size(); ++i) {
		if (extension == loc.cgi_ext[i])
			return true;
	}
	return false;
}

std::string CGIUtils::findInterpreter(const std::string& scriptPath, const Location& loc) {
	std::string extension = getFileExtension(scriptPath);
	if (extension == ".py") {
		for (size_t i = 0; i < loc.cgi_path.size(); ++i)
			if (loc.cgi_path[i].find("python") != std::string::npos)
				return loc.cgi_path[i];
		return "/usr/bin/python3";
	}
	if (extension == ".sh") {
		for (size_t i = 0; i < loc.cgi_path.size(); ++i)
			if (loc.cgi_path[i].find("bash") != std::string::npos)
				return loc.cgi_path[i];
		return "/bin/bash";
	}
	if (!loc.cgi_path.empty())
		return loc.cgi_path[0];
	return "";
}
