#include "../../../include/cgi/response/CGIResponseBuilder.hpp"
#include "../../../include/http/Response.hpp"
#include <sstream>
#include <cstdlib>

std::string CGIResponseBuilder::cleanLine(const std::string& line) {
	if (!line.empty() && line[line.size() - 1] == '\r') {
		return line.substr(0, line.size() - 1);
	}
	return line;
}

void CGIResponseBuilder::processHeaderLine(const std::string& line, Response& resp) {
	size_t colonPos = line.find(':');
	if (colonPos == std::string::npos) {
		return;
	}
	
	std::string headerName = line.substr(0, colonPos);
	std::string headerValue = line.substr(colonPos + 1);
	
	while (!headerValue.empty() && headerValue[0] == ' ') {
		headerValue.erase(0, 1);
	}
	
	if (headerName == "Content-Type") {
		resp.setContentType(headerValue);
	}
	else if (headerName == "Status") {
		int code = atoi(headerValue.c_str());
		if (code > 0) {
			resp.setStatus(code);
		}
	}
}

void CGIResponseBuilder::parseHeaders(const std::string& headers, Response& resp) {
	std::istringstream headerReader(headers);
	std::string line;
	
	resp.setStatus(200);
	
	while (std::getline(headerReader, line)) {
		line = cleanLine(line);
		processHeaderLine(line, resp);
	}
}

size_t CGIResponseBuilder::findHeaderEnd(const std::string& scriptOutput) {
	size_t headerEnd = scriptOutput.find("\r\n\r\n");
	if (headerEnd != std::string::npos) {
		return headerEnd + 4;
	}
	
	headerEnd = scriptOutput.find("\n\n");
	if (headerEnd != std::string::npos) {
		return headerEnd + 2;
	}

	return std::string::npos;
}

Response CGIResponseBuilder::buildResponse(const std::string& scriptOutput) {
	Response resp;
	
	size_t headerEnd = findHeaderEnd(scriptOutput);
	
	if (headerEnd == std::string::npos) {
		resp.setStatus(200);
		resp.setContentType("text/html");
		resp.setBody(scriptOutput);
		return resp;
	}
	
	std::string headers = scriptOutput.substr(0, headerEnd - 2);
	std::string body = scriptOutput.substr(headerEnd);
	
	parseHeaders(headers, resp);
	resp.setBody(body);
	
	return resp;
}