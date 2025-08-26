#include "./source/http_handler.cpp"
#include "./source/server_setup.cpp"

int main() {
    try {
        int server_fd = setup_server(8080);
        while (true)
        {
            int client_fd = accept_client(server_fd);
            //temp fix to socket reuse
            if (client_fd == -1) {
                usleep(1000);
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
