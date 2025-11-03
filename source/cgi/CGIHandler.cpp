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

#include "../../include/cgi/CGIHandler.hpp"
#include "../../include/configuration/configParse.hpp"
#include "../../include/cgi/CGIEnvironment.hpp"
#include "../../include/cgi/CGIProcess.hpp"
#include "../../include/cgi/CGIResponseBuilder.hpp"
#include "../../include/http/Request.hpp"
#include "../../include/http/Response.hpp"

#include <fstream>
#include <algorithm>
#include <cctype>
#include <iostream>

std::string CGIHandler::extNoDotLower(std::string p) {
	size_t dot = p.rfind('.');
	if (dot != std::string::npos) p = p.substr(dot + 1);
	for (size_t i = 0; i < p.size(); ++i)
		p[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(p[i])));
	return p;
}

bool CGIHandler::hasExt(const std::vector<std::string>& exts, const std::string& e) {
	for (size_t i = 0; i < exts.size(); ++i) {
		std::string ext = exts[i];
		if (!ext.empty() && ext[0] == '.') ext = ext.substr(1);
		for (size_t j = 0; j < ext.size(); ++j)
			ext[j] = static_cast<char>(std::tolower(static_cast<unsigned char>(ext[j])));
		if (ext == e) return true;
	}
	return false;
}

std::string CGIHandler::pickRunner(const Location& loc, const std::string& ext) {
	for (size_t i = 0; i < loc.cgi_ext.size(); ++i)
		if (ext == extNoDotLower(loc.cgi_ext[i])) return loc.cgi_path[i];
	return loc.cgi_path.empty() ? "" : loc.cgi_path[0];
}

CGIHandler::CGIHandler() {}
CGIHandler::~CGIHandler() {}

bool CGIHandler::canHandle(const Location& loc, const std::string& resolvedPath) const {
	std::string ext = extNoDotLower(resolvedPath);
	bool ok = hasExt(loc.cgi_ext, ext);
	return ok;
}
