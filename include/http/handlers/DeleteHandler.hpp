#ifndef DELETE_HANDLER_HPP
#define DELETE_HANDLER_HPP

#include <string>
#include "../Response.hpp"

class DeleteHandler {
public:
	static Response handleDelete(const std::string& filePath);
};

#endif
