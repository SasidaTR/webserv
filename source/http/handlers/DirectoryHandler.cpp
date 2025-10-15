#include "../../../include/http/handlers/DirectoryHandler.hpp"
#include <sys/stat.h>
#include <dirent.h>
#include <sstream>
#include <vector>
#include <algorithm>

bool DirectoryHandler::isDirectory(const std::string& path) {
	struct stat statbuf;
	if (stat(path.c_str(), &statbuf) != 0)
		return false;
	return S_ISDIR(statbuf.st_mode);
}

std::string DirectoryHandler::generateHTMLHeader(const std::string& urlPath) {
	std::ostringstream html;
	
	html << "<!DOCTYPE html>\n<html>\n<head>\n";
	html << "<meta charset=\"UTF-8\">\n";
	html << "<title>Index of " << urlPath << "</title>\n";
	html << "<style>\n";
	html << "body { font-family: Arial, sans-serif; margin: 40px; }\n";
	html << "h1 { color: #333; }\n";
	html << "table { border-collapse: collapse; width: 100%; }\n";
	html << "th, td { text-align: left; padding: 12px; border-bottom: 1px solid #ddd; }\n";
	html << "th { background-color: #4CAF50; color: white; }\n";
	html << "tr:hover { background-color: #f5f5f5; }\n";
	html << "a { color: #0066cc; text-decoration: none; }\n";
	html << "a:hover { text-decoration: underline; }\n";
	html << "</style>\n</head>\n<body>\n";
	html << "<h1>Index of " << urlPath << "</h1>\n";
	html << "<table>\n<thead>\n<tr><th>Name</th><th>Type</th></tr>\n</thead>\n<tbody>\n";
	
	return html.str();
}

std::string DirectoryHandler::generateHTMLFooter() {
	return "</tbody>\n</table>\n</body>\n</html>";
}

std::string DirectoryHandler::generateParentLink(const std::string& urlPath) {
	if (urlPath == "/")
		return "";
	return "<tr><td><a href=\"../\">../</a></td><td>Directory</td></tr>\n";
}

std::string DirectoryHandler::generateListing(const std::string& dirPath, const std::string& urlPath) {
	std::ostringstream html;
	
	html << generateHTMLHeader(urlPath);
	html << generateParentLink(urlPath);
	
	DIR* dir = opendir(dirPath.c_str());
	if (dir) {
		struct dirent* entry;
		std::vector<std::string> directories;
		std::vector<std::string> files;
		
		while ((entry = readdir(dir)) != NULL) {
			std::string name = entry->d_name;
			if (name == "." || name == "..")
				continue;
			
			std::string fullPath = dirPath + "/" + name;
			if (isDirectory(fullPath)) {
				directories.push_back(name);
			} else {
				files.push_back(name);
			}
		}
		closedir(dir);
		
		std::sort(directories.begin(), directories.end());
		std::sort(files.begin(), files.end());
		
		for (size_t i = 0; i < directories.size(); ++i) {
			html << "<tr><td><a href=\"" << directories[i] << "/\">" 
				<< directories[i] << "/</a></td><td>Directory</td></tr>\n";
		}
		
		for (size_t i = 0; i < files.size(); ++i) {
			html << "<tr><td><a href=\"" << files[i] << "\">" 
				<< files[i] << "</a></td><td>File</td></tr>\n";
		}
	}
	
	html << generateHTMLFooter();
	return html.str();
}
