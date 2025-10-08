#include "../../include/http/Router.hpp"
#include "../../include/http/handlers/StaticFileHandler.hpp"
#include "../../include/http/handlers/UploadHandler.hpp"
#include "../../include/http/handlers/DeleteHandler.hpp"
#include "../../include/http/handlers/DirectoryHandler.hpp"

Router::Router(const ServerFlat& s) : server(s) {}

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

bool Router::isMethodAllowed(const Request& req, const Location* loc) const {
	if (loc && !loc->loc_methods.empty()) {
		for (size_t i = 0; i < loc->loc_methods.size(); ++i) {
			if (loc->loc_methods[i] == req.getMethod()) {
				return true;
			}
		}
		return false;
	}
	
	if (!loc && req.getMethod() != "GET") {
		return false;
	}
	
	return true;
}

std::string Router::resolvePath(const Request& req, const Location* loc) const {
	std::string root = loc ? loc->root : server.root;
	std::string index = loc ? loc->index : server.index;
	std::string target = req.getTarget();
	
	size_t queryPos = target.find('?');
	if (queryPos != std::string::npos) {
		target = target.substr(0, queryPos);
	}
	
	if (target == "/" || (loc && target == loc->path)) {
		return root + "/" + index;
	}
	
	return root + target;
}

Response Router::route(const Request& req) const {
	Response resp;

	std::string target = req.getTarget();
	size_t queryPos = target.find('?');
	std::string pathOnly = (queryPos != std::string::npos) ? target.substr(0, queryPos) : target;

	const Location* loc = findMatchingLocation(pathOnly);

	if (loc && !loc->redirect_url.empty()) {
		resp.setStatus(loc->redirect_code > 0 ? loc->redirect_code : 301);
		resp.setRedirect(loc->redirect_url);
		return resp;
	}

	if (loc && cgiHandler.canHandle(req, *loc)) {
		return cgiHandler.execute(req, *loc);
	}

	if (!isMethodAllowed(req, loc)) {
		resp.setStatus(405);
		resp.setErrorBody(405);
		return resp;
	}

	std::string path = resolvePath(req, loc);

	if (req.getMethod() == "POST") {
		return UploadHandler::handleUpload(req.getBody(), path, loc, pathOnly);
	}
	
	if (req.getMethod() == "DELETE") {
		return DeleteHandler::handleDelete(path);
	}

	if (DirectoryHandler::isDirectory(path)) {
		std::string index = loc ? loc->index : server.index;
		std::string indexPath = path + "/" + index;
		
		std::string body;
		if (StaticFileHandler::readFile(indexPath, body)) {
			resp.setStatus(200);
			resp.setContentType(StaticFileHandler::getContentType(indexPath));
			resp.setBody(body);
			return resp;
		}
		
		if (loc && loc->autoindex) {
			std::string listing = DirectoryHandler::generateListing(path, pathOnly);
			resp.setStatus(200);
			resp.setContentType("text/html");
			resp.setBody(listing);
			return resp;
		}
		
		resp.setStatus(403);
		resp.setErrorBody(403);
		return resp;
	}

	return StaticFileHandler::serveFile(path);
}
