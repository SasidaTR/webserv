#include "./source/http_handler.cpp"
#include "./source/server_setup.cpp"

int main() {
	try {
		int server_fd = setup_server(8080);
		while (true)
		{
			std::vector<int> client_sockets;
			int client_fd = accept(server_fd, NULL, NULL);
			if (client_fd == -1) {
				if (errno == EAGAIN || errno == EWOULDBLOCK) {
					return -1;
				}
				throw std::runtime_error("accept failed");
			}
			client_sockets.push_back(client_fd);
			if (client_fd == -1) {
				continue;
			}
			handle_client(client_fd);
		}
		close(server_fd);
	} catch (const std::exception& e) {
		std::cerr << "Fatal: " << e.what() << "\n";
		return 1;
	}
	return 0;
}
