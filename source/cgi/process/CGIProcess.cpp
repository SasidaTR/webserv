#include "../../../include/cgi/process/CGIProcess.hpp"
#include "../../../include/cgi/utils/CGIEnvironment.hpp"
#include <sys/wait.h>
#include <unistd.h>
#include <cstdlib>

std::string CGIProcess::createErrorResponse(const std::string& message) {
	return "Content-Type: text/html\r\n\r\n<h1>Erreur CGI</h1><p>" + message + "</p>";
}

bool CGIProcess::createPipes(int pipeIn[2], int pipeOut[2]) {
	return (pipe(pipeIn) != -1 && pipe(pipeOut) != -1);
}

void CGIProcess::closePipes(int pipeIn[2], int pipeOut[2]) {
	close(pipeIn[0]); close(pipeIn[1]);
	close(pipeOut[0]); close(pipeOut[1]);
}

void CGIProcess::setupChildProcess(int pipeIn[2], int pipeOut[2]) {
	close(pipeIn[1]);
	close(pipeOut[0]);

	dup2(pipeIn[0], 0);
	dup2(pipeOut[1], 1);
	dup2(pipeOut[1], 2);
	
	close(pipeIn[0]);
	close(pipeOut[1]);
}

void CGIProcess::executeScript(const std::string& interpreter, const std::string& scriptPath, 
                              const std::vector<char*>& envArray) {
	char* args[] = {
		const_cast<char*>(interpreter.c_str()),
		const_cast<char*>(scriptPath.c_str()),
		NULL
	};
	
	execve(interpreter.c_str(), args, const_cast<char**>(&envArray[0]));
	exit(127);
}

std::string CGIProcess::handleParentProcess(int pipeIn[2], int pipeOut[2], 
                                           const std::string& inputData, pid_t childPid) {
	close(pipeIn[0]);
	close(pipeOut[1]);
	
	if (!inputData.empty()) {
		write(pipeIn[1], inputData.c_str(), inputData.size());
	}
	close(pipeIn[1]);
	
	std::string result;
	char buffer[1024];
	ssize_t bytesRead;
	
	while ((bytesRead = read(pipeOut[0], buffer, sizeof(buffer) - 1)) > 0) {
		buffer[bytesRead] = '\0';
		result += buffer;
	}
	close(pipeOut[0]);
	
	int status;
	waitpid(childPid, &status, 0);
	
	return result;
}

std::string CGIProcess::runScript(const std::string& interpreter, const std::string& scriptPath,
                                 const std::map<std::string, std::string>& environment, 
                                 const std::string& inputData) {
	int pipeIn[2], pipeOut[2];
	if (!createPipes(pipeIn, pipeOut)) {
		return createErrorResponse("Impossible de créer les tubes");
	}
	
	std::vector<char*> envArray = CGIEnvironment::createEnvArray(environment);
	
	pid_t childPid = fork();
	if (childPid == -1) {
		closePipes(pipeIn, pipeOut);
		return createErrorResponse("Impossible de créer le processus");
	}
	
	if (childPid == 0) {
		setupChildProcess(pipeIn, pipeOut);
		executeScript(interpreter, scriptPath, envArray);
	}
	
	return handleParentProcess(pipeIn, pipeOut, inputData, childPid);
}