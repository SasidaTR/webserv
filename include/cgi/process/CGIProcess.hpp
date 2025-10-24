#ifndef CGI_PROCESS_HPP
#define CGI_PROCESS_HPP

#include <string>
#include <map>
#include <vector>
#include <sys/types.h>

class CGIProcess {
public:
	std::string static runScript(const std::string& interpreter, const std::string& scriptPath,
								const std::map<std::string, std::string>& environment,
								const std::string& inputData);

private:
	static bool createPipes(int pipeIn[2], int pipeOut[2]);
	static void closePipes(int pipeIn[2], int pipeOut[2]);
	static void setupChildProcess(int pipeIn[2], int pipeOut[2]);
	static void executeScript(const std::string& interpreter, const std::string& scriptPath,
							const std::vector<char*>& envArray);
	static std::string handleParentProcess(int pipeIn[2], int pipeOut[2],
										const std::string& inputData, pid_t childPid);
	static std::string createErrorResponse(const std::string& message);
};

#endif
