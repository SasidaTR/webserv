/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jvittoz <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/02 19:49:00 by jvittoz           #+#    #+#             */
/*   Updated: 2025/11/02 19:49:04 by jvittoz          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../include/http/handlers/StaticFileHandler.hpp"
#include <fstream>
#include <sstream>

bool StaticFileHandler::readFile(const std::string& path, std::string& content) {
	std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
	if (!file)
		return false;
	
	std::ostringstream ss;
	ss << file.rdbuf();
	content = ss.str();
	return true;
}

std::string StaticFileHandler::getContentType(const std::string& path) {
	// HTML
	if (path.size() >= 5 && path.substr(path.size() - 5) == ".html")
		return "text/html";
	if (path.size() >= 4 && path.substr(path.size() - 4) == ".htm")
		return "text/html";
	
	// CSS
	if (path.size() >= 4 && path.substr(path.size() - 4) == ".css")
		return "text/css";
	
	// JavaScript
	if (path.size() >= 3 && path.substr(path.size() - 3) == ".js")
		return "application/javascript";
	
	// Images
	if (path.size() >= 4 && path.substr(path.size() - 4) == ".png")
		return "image/png";
	if (path.size() >= 4 && path.substr(path.size() - 4) == ".jpg")
		return "image/jpeg";
	if (path.size() >= 5 && path.substr(path.size() - 5) == ".jpeg")
		return "image/jpeg";
	if (path.size() >= 4 && path.substr(path.size() - 4) == ".gif")
		return "image/gif";
	if (path.size() >= 4 && path.substr(path.size() - 4) == ".ico")
		return "image/x-icon";
	
	// Text
	if (path.size() >= 4 && path.substr(path.size() - 4) == ".txt")
		return "text/plain";
	
	// Default
	return "application/octet-stream";
}

Response StaticFileHandler::serveFile(const std::string& path) {
	Response resp;
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
