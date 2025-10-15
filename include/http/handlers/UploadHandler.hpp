#ifndef UPLOAD_HANDLER_HPP
#define UPLOAD_HANDLER_HPP

#include <string>
#include "../Response.hpp"
#include "../../configuration/configParse.hpp"

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
