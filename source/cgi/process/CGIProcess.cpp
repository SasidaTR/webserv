#include "../../../include/cgi/process/CGIProcess.hpp"
#include "../../../include/cgi/utils/CGIEnvironment.hpp"
#include <sys/wait.h>
#include <unistd.h>
#include <cstdlib>
#include <fcntl.h>
#include <poll.h>
#include <cstring>
#include <errno.h>
#include <sstream>

std::string CGIProcess::createErrorResponse(const std::string& message) {
	return "Status: 500\r\nContent-Type: text/html\r\n\r\n<h1>Erreur CGI 500</h1><p>" + message + "</p>";
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
	
	fcntl(pipeIn[1], F_SETFL, O_NONBLOCK);
	fcntl(pipeOut[0], F_SETFL, O_NONBLOCK);
	
	if (!inputData.empty()) {
		size_t written = 0;
		while (written < inputData.size()) {
			struct pollfd pfd;
			pfd.fd = pipeIn[1];
			pfd.events = POLLOUT;
			int ret = poll(&pfd, 1, 5000);
			if (ret <= 0) break;
			
			ssize_t n = write(pipeIn[1], inputData.c_str() + written, inputData.size() - written);
			if (n > 0) written += n;
			else if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK) break;
		}
	}
	close(pipeIn[1]);
	
	std::string result;
	char buffer[1024];
	
	while (true) {
		struct pollfd pfd;
		pfd.fd = pipeOut[0];
		pfd.events = POLLIN;
		int ret = poll(&pfd, 1, 5000);
		if (ret <= 0) break;
		
		ssize_t bytesRead = read(pipeOut[0], buffer, sizeof(buffer) - 1);
		if (bytesRead > 0) {
			buffer[bytesRead] = '\0';
			result += buffer;
		} else if (bytesRead == 0) {
			break;
		} else if (errno != EAGAIN && errno != EWOULDBLOCK) {
			break;
		}
	}
	close(pipeOut[0]);
	
	int status;
	waitpid(childPid, &status, 0);
	
	if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
		std::ostringstream oss;
		oss << "Le script CGI s'est terminé avec une erreur (code: " << WEXITSTATUS(status) << ")";
		return createErrorResponse(oss.str());
	}
	if (WIFSIGNALED(status)) {
		std::ostringstream oss;
		oss << "Le script CGI a été tué par un signal " << WTERMSIG(status);
		return createErrorResponse(oss.str());
	}
	
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