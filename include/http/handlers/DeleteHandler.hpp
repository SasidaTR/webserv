#ifndef DELETE_HANDLER_HPP
#define DELETE_HANDLER_HPP

#include <string>
#include "../Response.hpp"

/**
 * DeleteHandler - Gestion de la suppression de fichiers (DELETE)
 * 
 * Responsabilités :
 * - Supprimer des fichiers du disque
 * - Générer des réponses HTTP appropriées
 * - Gérer les erreurs de suppression
 */
class DeleteHandler {
public:
	static Response handleDelete(const std::string& filePath);
};

#endif
