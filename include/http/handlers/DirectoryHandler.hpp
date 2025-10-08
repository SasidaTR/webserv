#ifndef DIRECTORY_HANDLER_HPP
#define DIRECTORY_HANDLER_HPP

#include <string>

/**
 * DirectoryHandler - Gestion de l'autoindex et du directory listing
 * 
 * Responsabilités :
 * - Vérifier si un chemin est un répertoire
 * - Générer des listings HTML de répertoires
 * - Gérer l'affichage stylé des fichiers et dossiers
 */
class DirectoryHandler {
public:
	static bool isDirectory(const std::string& path);
	static std::string generateListing(const std::string& dirPath, const std::string& urlPath);
	
private:
	static std::string generateHTMLHeader(const std::string& urlPath);
	static std::string generateHTMLFooter();
	static std::string generateParentLink(const std::string& urlPath);
};

#endif
