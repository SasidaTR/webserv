#include "../../include/cgi/CGIHandler.hpp"
#include "../../include/configuration/configParse.hpp"   // ✅ add this line
#include "../../include/cgi/CGIUtils.hpp"
#include "../../include/cgi/CGIEnvironment.hpp"
#include "../../include/cgi/CGIProcess.hpp"
#include "../../include/cgi/CGIResponseBuilder.hpp"
#include "../../include/http/Request.hpp"
#include "../../include/http/Response.hpp"


#include <fstream>
#include <algorithm>
#include <cctype>

// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------

std::string CGIHandler::extNoDotLower(std::string p) {
    size_t dot = p.rfind('.');
    if (dot != std::string::npos)
        p = p.substr(dot + 1);
    for (size_t i = 0; i < p.size(); ++i)
        p[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(p[i])));
    return p;
}

bool CGIHandler::hasExt(const std::vector<std::string>& exts, const std::string& e) {
    for (size_t i = 0; i < exts.size(); ++i) {
        std::string ext = exts[i];
        if (!ext.empty() && ext[0] == '.')
            ext = ext.substr(1);
        std::string low = ext;
        for (size_t j = 0; j < low.size(); ++j)
            low[j] = static_cast<char>(std::tolower(static_cast<unsigned char>(low[j])));
        if (low == e)
            return true;
    }
    return false;
}

std::string CGIHandler::pickRunner(const Location& loc, const std::string& ext) {
    if (loc.cgi_ext.empty() || loc.cgi_path.empty())
        return "";

    // Find the interpreter that matches the extension
    for (size_t i = 0; i < loc.cgi_ext.size(); ++i) {
        if (ext == loc.cgi_ext[i]) {
            if (i < loc.cgi_path.size())
                return loc.cgi_path[i];
            else
                return ""; // no matching path
        }
    }
    return ""; // extension not found
}


// -----------------------------------------------------------------------------
// Main logic
// -----------------------------------------------------------------------------

CGIHandler::CGIHandler() {}
CGIHandler::~CGIHandler() {}

bool CGIHandler::canHandle(const Request& req, const Location& loc, const std::string& resolvedPath) const {
    (void)resolvedPath;
    std::string ext = extNoDotLower(req.getTarget());
	std::cerr << "[CGI canHandle] url=" << req.getTarget()
          << " fs=" << resolvedPath
          << " ext=" << extNoDotLower(resolvedPath) << " → " 
          << (hasExt(loc.cgi_ext, extNoDotLower(resolvedPath)) ? "yes" : "no") << "\n";

    return hasExt(loc.cgi_ext, ext);
}