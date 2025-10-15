#include "../../include/cgi/CGIHandler.hpp"
#include "../../include/cgi/utils/CGIUtils.hpp"
#include "../../include/cgi/utils/CGIEnvironment.hpp"
#include "../../include/cgi/process/CGIProcess.hpp"
#include "../../include/cgi/response/CGIResponseBuilder.hpp"
#include "../../include/http/Request.hpp"
#include "../../include/http/Response.hpp"
#include "../../include/configuration/configParse.hpp"
#include <fstream>

/**
 * CGIHandler - Orchestrateur principal pour la gestion des scripts CGI
 */

CGIHandler::CGIHandler() {
}

CGIHandler::~CGIHandler() {
}

bool CGIHandler::canHandle(const Request& req, const Location& loc) const {
	return CGIUtils::isScriptFile(req.getTarget(), loc);
}

Response CGIHandler::execute(const Request& req, const Location& loc) const {
	std::string url = req.getTarget();
	std::string scriptPath = CGIUtils::cleanUrl(url);
	std::string fullScriptPath = loc.root + scriptPath;
	
	std::ifstream file(fullScriptPath.c_str());
	if (!file.good()) {
		Response resp;
		resp.setStatus(404);
		resp.setErrorBody(404);
		return resp;
	}
	file.close();
	
	std::string interpreter = CGIUtils::findInterpreter(fullScriptPath, loc);
	if (interpreter.empty()) {
		Response resp;
		resp.setStatus(500);
		resp.setContentType("text/html");
		resp.setBody("<h1>Erreur 500</h1><p>Pas d'interpréteur trouvé pour ce script</p>");
		return resp;
	}
	
	std::map<std::string, std::string> env = CGIEnvironment::prepare(req, fullScriptPath);
	
	std::string scriptOutput = CGIProcess::runScript(interpreter, fullScriptPath, env, req.getBody());
	
	return CGIResponseBuilder::buildResponse(scriptOutput);
}