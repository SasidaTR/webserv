#include "../../../include/http/handlers/UploadHandler.hpp"
#include <fstream>

std::string UploadHandler::extractFilename(const std::string& urlPath) {
	size_t lastSlash = urlPath.find_last_of('/');
	if (lastSlash != std::string::npos && lastSlash + 1 < urlPath.size()) {
		return urlPath.substr(lastSlash + 1);
	}
	return "upload_file";
}

std::string UploadHandler::determineUploadPath(const std::string& defaultPath,
                                               const Location* loc,
                                               const std::string& urlPath) {
	if (loc && !loc->upload_dir.empty()) {
		std::string filename = extractFilename(urlPath);
		return loc->upload_dir + "/" + filename;
	}
	
	return defaultPath;
}

Response UploadHandler::handleUpload(const std::string& body,
                                     const std::string& defaultPath,
                                     const Location* loc,
                                     const std::string& urlPath) {
	Response resp;
	
	std::string uploadPath = determineUploadPath(defaultPath, loc, urlPath);
	
	std::ofstream outFile(uploadPath.c_str(), std::ios::binary);
	if (!outFile) {
		resp.setStatus(500);
		resp.setErrorBody(500);
		return resp;
	}
	
	outFile << body;
	outFile.close();
	
	resp.setStatus(201);
	resp.setBody("<h1>Created</h1><p>File uploaded successfully to " + uploadPath + "</p>");
	resp.setContentType("text/html");
	
	return resp;
}
