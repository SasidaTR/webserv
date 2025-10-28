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
	(void)ext;

    if (loc.cgi_path.empty())
        return "";
    // Very simple: return first defined runner (cgi_path[0])
    // For example: /usr/bin/python3 or ./ubuntu_cgi_tester
    return loc.cgi_path[0];
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

Response CGIHandler::execute(const Request& req, const Location& loc, const std::string& resolvedPath) const {
    Response resp;

    // 404 if target resource (the .bla file) doesn't exist
    std::ifstream file(resolvedPath.c_str());
    if (!file.good()) {
        resp.setStatus(404);
        resp.setErrorBody(404);
        return resp;
    }
    file.close();

    // Pick the CGI runner (executable), e.g. ./ubuntu_cgi_tester
    std::string ext    = extNoDotLower(resolvedPath);
    std::string runner = pickRunner(loc, ext);
    if (runner.empty()) {
        resp.setStatus(500);
        resp.setContentType("text/html");
        resp.setBody("<h1>500 Internal Server Error</h1><p>No CGI runner found.</p>");
        return resp;
    }

    // Make both runner and resource absolute
    std::string runnerAbs = runner;
    {
        char buf[1024];
        if (realpath(runner.c_str(), buf)) runnerAbs = std::string(buf);
    }

    std::string absResourcePath = resolvedPath;
    {
        char buf[1024];
        if (realpath(resolvedPath.c_str(), buf)) absResourcePath = std::string(buf);
    }

    // URL path (no query) for SCRIPT_NAME
    std::string urlPathOnly = req.getTarget();
    size_t q = urlPathOnly.find('?');
    if (q != std::string::npos) urlPathOnly = urlPathOnly.substr(0, q);

    // Build base env (will override key vars just below)
    std::map<std::string, std::string> env = CGIEnvironment::prepare(req, runnerAbs);

    // === Tester-expected mapping for extension-based CGI (.bla) ===
    // Treat the requested .bla file as "the script"
	env["SCRIPT_FILENAME"]  = absResourcePath;   // FS path to .bla
	env["PATH_INFO"]        = urlPathOnly;       // URL path (NOT a FS path)
	env["PATH_TRANSLATED"]  = absResourcePath;   // FS path translation of PATH_INFO
	// keep:
	env["SCRIPT_NAME"]      = urlPathOnly;


    // --- optional debug (keep/remove as you like) ---
    std::cerr << "\n=== CGI DEBUG (parent) ===\n";
    std::cerr << "req.method       = " << req.getMethod() << "\n";
    std::cerr << "req.target       = " << req.getTarget() << "\n";
    std::cerr << "location.path    = " << loc.path << "\n";
    std::cerr << "resolvedPath(FS) = " << resolvedPath << "\n";
    std::cerr << "runnerAbs        = " << runnerAbs << "\n";
    std::cerr << "absResourcePath  = " << absResourcePath << "\n";
    std::cerr << "--- ENV we will pass to execve ---\n";
    for (std::map<std::string,std::string>::const_iterator it = env.begin(); it != env.end(); ++it)
        std::cerr << it->first << "=" << it->second << "\n";
    std::cerr << "=== END CGI DEBUG ===\n" << std::endl;
    // -----------------------------------------------

    // Run CGI: argv[0] = runnerAbs, argv[1] = absResourcePath
    std::string scriptOutput = CGIProcess::runScript(runnerAbs, absResourcePath, env, req.getBody());

    return CGIResponseBuilder::buildResponse(scriptOutput);
}


