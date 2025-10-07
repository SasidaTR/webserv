#ifndef CGI_ENVIRONMENT_HPP
#define CGI_ENVIRONMENT_HPP

#include <string>
#include <map>
#include <vector>

// Forward declarations
class Request;

/**
 * CGIEnvironment - Gestion de l'environnement CGI
 * 
 * Responsabilités :
 * - Préparer les variables d'environnement pour les scripts CGI
 * - Convertir en format C pour execve()
 */
class CGIEnvironment {
public:
    static std::map<std::string, std::string> prepare(const Request& req, const std::string& scriptPath);
    
    static std::vector<char*> createEnvArray(const std::map<std::string, std::string>& environment);

private:
    static void addBasicVariables(std::map<std::string, std::string>& env, const Request& req, const std::string& scriptPath);
    static void addUrlVariables(std::map<std::string, std::string>& env, const Request& req);
    static void addHttpHeaders(std::map<std::string, std::string>& env, const Request& req);
    static void addServerDefaults(std::map<std::string, std::string>& env);
};

#endif