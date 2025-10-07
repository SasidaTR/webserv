#ifndef CGI_UTILS_HPP
#define CGI_UTILS_HPP

#include <string>
#include <vector>

// Forward declarations
struct Location;

/**
 * CGIUtils - Utilitaires de base pour le CGI
 * 
 * Fonctions simples pour :
 * - Détecter les extensions de fichiers
 * - Vérifier si un fichier est un script CGI
 * - Trouver le bon interpréteur
 */
class CGIUtils {
public:
    static std::string getFileExtension(const std::string& filename);
    
    static bool isScriptFile(const std::string& path, const Location& loc);
    
    static std::string findInterpreter(const std::string& scriptPath, const Location& loc);
    
    static std::string cleanUrl(const std::string& url);
};

#endif