#include "../../../include/cgi/utils/CGIUtils.hpp"
#include "../../../include/configuration/configParse.hpp"

// Trouve l'extension d'un fichier (ex: ".py" pour "script.py")
std::string CGIUtils::getFileExtension(const std::string& filename) {
	size_t dotPos = filename.find_last_of('.');
	
	if (dotPos == std::string::npos || dotPos == filename.length() - 1) {
		return "";
	}
	
	return filename.substr(dotPos);
}

// Nettoie une URL (enlève les paramètres après le ?)
std::string CGIUtils::cleanUrl(const std::string& url) {
	size_t queryPos = url.find('?');
	if (queryPos != std::string::npos) {
		return url.substr(0, queryPos);
	}
	return url;
}

// Vérifie si un fichier est un script CGI selon la configuration
bool CGIUtils::isScriptFile(const std::string& path, const Location& loc) {
	if (loc.cgi_ext.empty()) {
		return false;
	}
	
	std::string cleanPath = cleanUrl(path);
	std::string extension = getFileExtension(cleanPath);
	
	for (size_t i = 0; i < loc.cgi_ext.size(); ++i) {
		if (extension == loc.cgi_ext[i]) {
			return true;
		}
	}
	
	return false;
}

// Trouve le bon interpréteur pour exécuter le script
std::string CGIUtils::findInterpreter(const std::string& scriptPath, const Location& loc) {
	std::string extension = getFileExtension(scriptPath);
	
	// Scripts Python (.py)
	if (extension == ".py") {
		for (size_t i = 0; i < loc.cgi_path.size(); ++i) {
			if (loc.cgi_path[i].find("python") != std::string::npos) {
				return loc.cgi_path[i];
			}
		}
		return "/usr/bin/python3";
	}
	
	// Scripts Bash (.sh)  
	if (extension == ".sh") {
		for (size_t i = 0; i < loc.cgi_path.size(); ++i) {
			if (loc.cgi_path[i].find("bash") != std::string::npos) {
				return loc.cgi_path[i];
			}
		}
		return "/bin/bash";
	}
	
	// Interpréteur par défaut
	if (!loc.cgi_path.empty()) {
		return loc.cgi_path[0];
	}
	
	return "";
}