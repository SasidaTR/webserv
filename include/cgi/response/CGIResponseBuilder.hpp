#ifndef CGI_RESPONSE_HPP
#define CGI_RESPONSE_HPP

#include <string>

class Response;

/**
 * CGIResponseBuilder - Construction de réponses HTTP à partir de sorties CGI
 * 
 * Responsabilités :
 * - Parser la sortie brute des scripts CGI
 * - Séparer headers et body
 * - Construire une réponse HTTP valide
 */
class CGIResponseBuilder {
public:
    static Response buildResponse(const std::string& scriptOutput);

private:
    static size_t findHeaderEnd(const std::string& scriptOutput);
    
    static void parseHeaders(const std::string& headers, Response& resp);
    
    static void processHeaderLine(const std::string& line, Response& resp);
    
    static std::string cleanLine(const std::string& line);
};

#endif