#include "./source/http_handler.cpp"
#include "./source/server_setup.cpp"
#include <csignal>
#include <vector>
#include <poll.h>
#include <unistd.h>
#include <iostream>
#include <stdexcept>


int main() {
    try {
        std::signal(SIGPIPE, SIG_IGN);
        int server_fd = setup_server(8080);
        std::vector<pollfd> fds;
		struct pollfd p;
		p.fd = server_fd;
		p.events = POLLIN;
		p.revents = 0;
		fds.push_back(p);

        while (true) {
			int temp = poll(fds.data(), fds.size(), 0);
			if (temp == -1)
				throw std::runtime_error("could not poll()");
			for (size_t i = 0; i < fds.size(); i++)
			{
				pollfd &ptr = fds[i];
				if (ptr.revents == 0)
					continue;
				--temp;
				
				if (ptr.fd == server_fd && (ptr.revents& POLLIN)){
					int client_fd = accept(server_fd, NULL, NULL);
					if (client_fd != -1){
						struct pollfd p;
						p.fd = client_fd;
						p.events = POLLIN;
						p.revents = 0;
						fds.push_back(p);
					}
					continue;
				}

				if (ptr.revents& POLLIN){
					char buffer[1024];
					size_t n = read(ptr.fd, buffer, sizeof(buffer));
					if (n <= 0){
						close(ptr.fd);
						fds[i] = fds.back();
						fds.pop_back();
						i--;
					}
					else{
						std::cout << std::string(buffer, n) << std::endl;
						ptr.events |= POLLOUT;
					}
				}
				if (ptr.revents & POLLIN) {
					char buf[1024];
					ssize_t n = read(ptr.fd, buf, sizeof(buf));
					if (n <= 0) {
						close(ptr.fd);
						fds[i] = fds.back();
						fds.pop_back();
						--i;
					} 
					else {
						std::cout << "Received: " << std::string(buf, n) << "\n";
						ptr.events |= POLLOUT;
					}
            	}
			}
        }
    	close(server_fd);
    } catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
