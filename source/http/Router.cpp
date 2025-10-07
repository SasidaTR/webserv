#include "../../include/http/Router.hpp"

Router::Router(const ServerFlat& s) : server(s) {}

std::string Router::getContentType(const std::string& path) const {
	if (path.size() >= 5 && path.substr(path.size() - 5) == ".html") return "text/html";
	if (path.size() >= 4 && path.substr(path.size() - 4) == ".css") return "text/css";
	if (path.size() >= 3 && path.substr(path.size() - 3) == ".js") return "application/javascript";
	if (path.size() >= 4 && path.substr(path.size() - 4) == ".png") return "image/png";
	if (path.size() >= 4 && path.substr(path.size() - 4) == ".jpg") return "image/jpeg";
	return "text/plain";
}

bool Router::readFile(const std::string& path, std::string& content) const {
	std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
	if (!file) return false;
	std::ostringstream ss;
	ss << file.rdbuf();
	content = ss.str();
	return true;
}

const Location* Router::findMatchingLocation(const std::string& path) const {
	const Location* bestMatch = NULL;
	size_t bestMatchLen = 0;
	
	for (size_t i = 0; i < server.locations.size(); ++i) {
		const Location& loc = server.locations[i];
		const std::string& locPath = loc.path;
		
		if (path.size() >= locPath.size() && 
		    path.substr(0, locPath.size()) == locPath) {
			if (path.size() == locPath.size() || 
			    (path.size() > locPath.size() && (path[locPath.size()] == '/' || locPath[locPath.size()-1] == '/'))) {
				if (locPath.size() > bestMatchLen) {
					bestMatch = &loc;
					bestMatchLen = locPath.size();
				}
			}
		}
	}
	
	return bestMatch;
}

Response Router::route(const Request& req) const {
	Response resp;

	std::string target = req.getTarget();
	size_t queryPos = target.find('?');
	std::string pathOnly = (queryPos != std::string::npos) ? target.substr(0, queryPos) : target;
	
	const Location* loc = findMatchingLocation(pathOnly);
	
	if (loc && cgiHandler.canHandle(req, *loc)) {
		return cgiHandler.execute(req, *loc);
	}

	if (loc) {
		if (!loc->loc_methods.empty()) {
			bool methodAllowed = false;
			for (size_t i = 0; i < loc->loc_methods.size(); ++i) {
				if (loc->loc_methods[i] == req.getMethod()) {
					methodAllowed = true;
					break;
				}
			}
			if (!methodAllowed) {
				resp.setStatus(405);
				resp.setErrorBody(405);
				return resp;
			}
		}
	} else if (req.getMethod() != "GET") {
		resp.setStatus(405);
		resp.setErrorBody(405);
		return resp;
	}

	std::string root = loc ? loc->root : server.root;
	std::string index = loc ? loc->index : server.index;
	
	std::string path;
	if (req.getTarget() == "/" || (loc && req.getTarget() == loc->path))
		path = root + "/" + index;
	else
		path = root + req.getTarget();

	std::string body;
	if (readFile(path, body)) {
		resp.setStatus(200);
		resp.setContentType(getContentType(path));
		resp.setBody(body);
	} else {
		resp.setStatus(404);
		resp.setErrorBody(404);
	}

	return resp;
}
