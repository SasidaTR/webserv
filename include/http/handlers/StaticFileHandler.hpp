#ifndef STATIC_FILE_HANDLER_HPP
#define STATIC_FILE_HANDLER_HPP

#include <string>
#include "../Response.hpp"

/**
 * StaticFileHandler - Gestion des fichiers statiques
 * 
 * Responsabilités :
 * - Lire des fichiers depuis le disque
 * - Déterminer le Content-Type
 * - Construire des réponses HTTP pour fichiers statiques
 */
class StaticFileHandler {
public:
	static bool readFile(const std::string& path, std::string& content);
	static std::string getContentType(const std::string& path);
	static Response serveFile(const std::string& path);
};

#endif
