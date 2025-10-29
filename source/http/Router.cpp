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
		if (path.size() >= locPath.size() && path.substr(0, locPath.size()) == locPath) {
			if (path.size() == locPath.size() || (path.size() > locPath.size() && (path[locPath.size()] == '/' || locPath[locPath.size()-1] == '/'))) {
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
			if (loc->loc_methods[i] == req.getMethod())
				return true;
		}
		return false;
	}
	if (!loc && req.getMethod() != "GET" && req.getMethod() != "HEAD")
		return false;
	return true;
}

static size_t parseSize(const std::string& sizeStr) {
	if (sizeStr.empty())
		return 1048576; // 1MB default
	size_t value = 0;
	size_t i = 0;
	while (i < sizeStr.size() && sizeStr[i] >= '0' && sizeStr[i] <= '9') {
		value = value * 10 + (sizeStr[i] - '0');
		++i;
	}
	if (i < sizeStr.size()) {
		char unit = sizeStr[i];
		if (unit == 'K' || unit == 'k')
			value *= 1024;
		else if (unit == 'M' || unit == 'm')
			value *= 1024 * 1024;
		else if (unit == 'G' || unit == 'g')
			value *= 1024 * 1024 * 1024;
	}
	return value;
}

bool Router::checkBodySize(const Request& req, const Location* loc) const {
	std::string bodyLimitStr = loc ? loc->client_max_body_size : server.client_max_body_size;
	size_t maxSize = parseSize(bodyLimitStr);
	return req.getBody().size() <= maxSize;
}

std::string Router::resolvePath(const Request& req, const Location* loc) const {
	std::string uri = req.getTarget();

	size_t queryPos = uri.find('?');
	if (queryPos != std::string::npos)
		uri = uri.substr(0, queryPos);

	std::string index = (loc && !loc->index.empty()) ? loc->index : server.index;

	std::string base;
	if (loc && !loc->alias.empty())
		base = loc->alias;
	else if (loc && !loc->root.empty())
		base = loc->root;
	else
		base = server.root;

	std::string fullpath;
	if (loc && !loc->alias.empty()) {
		if (uri.find(loc->path) == 0)
			fullpath = base + uri.substr(loc->path.size());
		else
			fullpath = base + uri;
	} else {
		fullpath = base + uri;
	}
	return fullpath;
}

Response Router::route(const Request& req) const {
	Response resp;

	if (!req.isValidMethod()) {
		resp.setStatus(501);
		resp.setErrorBody(501);
		return resp;
	}

	std::string target = req.getTarget();
	size_t queryPos = target.find('?');
	std::string pathOnly = (queryPos != std::string::npos) ? target.substr(0, queryPos) : target;
	const Location* loc = findMatchingLocation(pathOnly);

	if (!checkBodySize(req, loc)) {
		resp.setStatus(413);
		resp.setErrorBody(413);
		return resp;
	}

	if (loc && !loc->redirect_url.empty()) {
		resp.setStatus(loc->redirect_code > 0 ? loc->redirect_code : 301);
		resp.setRedirect(loc->redirect_url);
		return resp;
	}

	if (!isMethodAllowed(req, loc)) {
		resp.setStatus(405);
		resp.setErrorBody(405);
		return resp;
	}

	std::string path = resolvePath(req, loc);


	// ICI DETECT LA CGI ET RENVOIE UNE REPONSE MINIMAL
	if (loc && cgiHandler.canHandle(req, *loc, path)) {
		const std::string interp = !loc->cgi_path.empty() ? loc->cgi_path[0] : std::string();
		resp.markAsCgi(path, interp);
		return resp;
	}


	// if (DirectoryHandler::isDirectory(path) && !target.empty() && target[target.size() - 1] != '/') {
	// 	resp.setStatus(301);
	// 	resp.setRedirect(target + "/");
	// 	return resp;
	// }

	if (req.getMethod() == "POST")
		return UploadHandler::handleUpload(req.getBody(), path, loc, pathOnly);
	if (req.getMethod() == "PUT")
		return UploadHandler::handleUpload(req.getBody(), path, loc, pathOnly);
	if (req.getMethod() == "DELETE")
		return DeleteHandler::handleDelete(path);

	if (DirectoryHandler::isDirectory(path)) {
		std::string index = loc ? loc->index : server.index;
		std::string indexPath = path + "/" + index;

		std::string body;
		if (StaticFileHandler::readFile(indexPath, body)) {
			resp.setStatus(200);
			resp.setContentType(StaticFileHandler::getContentType(indexPath));
			if (req.getMethod() != "HEAD")
				resp.setBody(body);
			return resp;
		}
		if (loc && loc->autoindex) {
			std::string listing = DirectoryHandler::generateListing(path, pathOnly);
			resp.setStatus(200);
			resp.setContentType("text/html");
			if (req.getMethod() != "HEAD")
				resp.setBody(listing);
			return resp;
		}
		resp.setStatus(404);
		resp.setErrorBody(404);
		return resp;
	}

	Response fileResp = StaticFileHandler::serveFile(path);
	if (req.getMethod() == "HEAD")
		fileResp.setBody("");
	return fileResp;
}
