#ifndef CGI_HANDLER_HPP
#define CGI_HANDLER_HPP

#include <string>

// Forward declarations
class Request;
class Response;
struct Location;

/**
 * CGIHandler - Interface principale pour la gestion des scripts CGI
 * 
 * Cette classe orchestre les différents modules CGI pour :
 * 1. Vérifier si une URL demande un script CGI
 * 2. Exécuter le script et retourner le résultat
 * 
 * ARCHITECTURE MODULAIRE :
 * - CGIUtils : Détection et validation des scripts
 * - CGIEnvironment : Préparation des variables d'environnement  
 * - CGIProcess : Exécution des processus et communication
 * - CGIResponseBuilder : Construction des réponses HTTP
 */
class CGIHandler {
public:
    CGIHandler();
    ~CGIHandler();
    
    bool canHandle(const Request& req, const Location& loc) const;
    Response execute(const Request& req, const Location& loc) const;
};

#endif