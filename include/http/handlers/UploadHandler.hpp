#ifndef UPLOAD_HANDLER_HPP
#define UPLOAD_HANDLER_HPP

#include <string>
#include "../Response.hpp"
#include "../../configuration/configParse.hpp"

/**
 * UploadHandler - Gestion des uploads de fichiers (POST)
 * 
 * Responsabilités :
 * - Déterminer le chemin de destination pour uploads
 * - Écrire les données uploadées sur le disque
 * - Générer des réponses HTTP appropriées
 */
class UploadHandler {
public:
	static Response handleUpload(const std::string& body, 
	                             const std::string& defaultPath,
	                             const Location* loc,
	                             const std::string& urlPath);
	
private:
	static std::string determineUploadPath(const std::string& defaultPath,
	                                      const Location* loc,
	                                      const std::string& urlPath);
	static std::string extractFilename(const std::string& urlPath);
};

#endif
