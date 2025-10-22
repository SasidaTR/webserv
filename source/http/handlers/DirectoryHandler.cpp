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
	html << "<!DOCTYPE html>\n"
		<< "<html lang=\"en\">\n"
		<< "  <head>\n"
		<< "    <meta charset=\"UTF-8\">\n"
		<< "    <title>webserv</title>\n"
		<< "    <link rel=\"stylesheet\" href=\"/style.css\">\n"
		<< "  </head>\n"
		<< "  <body>\n"
		<< "    <header>\n"
		<< "      <nav class=\"navbar\">\n"
		<< "        <p class=\"search\">webserv</p>\n"
		<< "        <button class=\"btn-danger\">Have a problem?</button>\n"
		<< "      </nav>\n"
		<< "    </header>\n"
		<< "    <aside>\n"
		<< "      <nav class=\"sidebar\">\n"
		<< "      </nav>\n"
		<< "    </aside>\n"
		<< "    <main>\n"
		<< "      <div class=\"first\">\n"
		<< "        <div class=\"card\">\n"
		<< "          <img class=\"photo\" src=\"/media/trischma.jpg\" alt=\"trischma\">\n"
		<< "          <div>\n"
		<< "            <h2>Tommy Rischmann</h2>\n"
		<< "            <p>trischma</p>\n"
		<< "          </div>\n"
		<< "        </div>\n"
		<< "        <div class=\"card\">\n"
		<< "          <img class=\"photo\" src=\"/media/douzgane.jpg\" alt=\"douzgane\">\n"
		<< "          <div>\n"
		<< "            <h2>Driss Ouzgane</h2>\n"
		<< "            <p>douzgane</p>\n"
		<< "          </div>\n"
		<< "        </div>\n"
		<< "        <div class=\"card\">\n"
		<< "          <img class=\"photo\" src=\"/media/jvittoz.jpg\" alt=\"jvittoz\">\n"
		<< "          <div>\n"
		<< "            <h2>Joseph Vittoz</h2>\n"
		<< "            <p>jvittoz</p>\n"
		<< "          </div>\n"
		<< "        </div>\n"
		<< "      </div>\n";

	DIR* dir;
	struct dirent* entry;
	std::vector<std::string> files;

	dir = opendir("./html/files");
	if (dir != NULL) {
		while ((entry = readdir(dir)) != NULL) {
			std::string name = entry->d_name;
			if (name != "." && name != "..")
				files.push_back(name);
		}
		closedir(dir);
	}

	std::sort(files.begin(), files.end());

	html << "      <div class=\"second\">\n";
	for (size_t i = 0; i < files.size(); ++i) {
		html << "        <a href=\"/files/" << files[i] 
			<< "\" class=\"btn-primary\">" << files[i] << "</a>\n";
	}
	html << "      </div>\n";

	html << "      <div class=\"test\">\n"
		<< "        <p class=\"texte\" style=\"font-size: 40px;\">" << urlPath << "</p>\n"
		<< "      </div>\n"
		<< "    </main>\n"
		<< "    <footer>\n"
		<< "      <p>&copy; 2025 webserv. All rights reserved.</p>\n"
		<< "    </footer>\n"
		<< "    <script src=\"/main.js\"></script>\n"
		<< "  </body>\n"
		<< "</html>\n";

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
