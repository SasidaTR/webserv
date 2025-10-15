#include "../../../include/http/handlers/DeleteHandler.hpp"
#include <cstdio>

Response DeleteHandler::handleDelete(const std::string& filePath) {
	Response resp;
	
	if (std::remove(filePath.c_str()) != 0) {
		resp.setStatus(500);
		resp.setErrorBody(500);
	} else {
		resp.setStatus(204);
		resp.setBody("");
	}
	
	return resp;
}
