#include "../../../include/cgi/utils/CGIEnvironment.hpp"
#include "../../../include/http/Request.hpp"

// Variables d'environnement de base
void CGIEnvironment::addBasicVariables(std::map<std::string, std::string>& env, const Request& req, const std::string& scriptPath) {
	env["REQUEST_METHOD"] = req.getMethod();
	env["SERVER_SOFTWARE"] = "webserv/1.0";
	env["SCRIPT_FILENAME"] = scriptPath;
	env["CONTENT_LENGTH"] = "0";  // Par défaut
}

// Variables liées à l'URL et aux paramètres
void CGIEnvironment::addUrlVariables(std::map<std::string, std::string>& env, const Request& req) {
	std::string url = req.getTarget();
	size_t queryPos = url.find('?');
	
	if (queryPos != std::string::npos) {
		env["SCRIPT_NAME"] = url.substr(0, queryPos);
		env["QUERY_STRING"] = url.substr(queryPos + 1);
	} else {
		env["SCRIPT_NAME"] = url;
		env["QUERY_STRING"] = "";
	}
}

// Headers HTTP importants
void CGIEnvironment::addHttpHeaders(std::map<std::string, std::string>& env, const Request& req) {
	const std::map<std::string, std::string>& headers = req.getHeaders();
	
	for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
		if (it->first == "Content-Length") {
			env["CONTENT_LENGTH"] = it->second;
		}
		else if (it->first == "Content-Type") {
			env["CONTENT_TYPE"] = it->second;
		}
		else if (it->first == "Host") {
			// Sépare host:port
			size_t colon = it->second.find(':');
			if (colon != std::string::npos) {
				env["SERVER_NAME"] = it->second.substr(0, colon);
				env["SERVER_PORT"] = it->second.substr(colon + 1);
			} else {
				env["SERVER_NAME"] = it->second;
				env["SERVER_PORT"] = "80";
			}
		}
	}
}

// Valeurs par défaut du serveur
void CGIEnvironment::addServerDefaults(std::map<std::string, std::string>& env) {
	if (env.find("SERVER_NAME") == env.end()) {
		env["SERVER_NAME"] = "localhost";
	}
	if (env.find("SERVER_PORT") == env.end()) {
		env["SERVER_PORT"] = "8080";
	}
}

// Prépare l'environnement complet
std::map<std::string, std::string> CGIEnvironment::prepare(const Request& req, const std::string& scriptPath) {
	std::map<std::string, std::string> env;
	
	addBasicVariables(env, req, scriptPath);
	addUrlVariables(env, req);
	addHttpHeaders(env, req);
	addServerDefaults(env);
	
	return env;
}

// Convertit en tableau C pour execve()
std::vector<char*> CGIEnvironment::createEnvArray(const std::map<std::string, std::string>& environment) {
	static std::vector<std::string> envStrings;
	envStrings.clear();
	
	for (std::map<std::string, std::string>::const_iterator it = environment.begin(); it != environment.end(); ++it) {
		envStrings.push_back(it->first + "=" + it->second);
	}
	
	std::vector<char*> envArray;
	for (size_t i = 0; i < envStrings.size(); ++i) {
		envArray.push_back(const_cast<char*>(envStrings[i].c_str()));
	}
	envArray.push_back(NULL);
	
	return envArray;
}