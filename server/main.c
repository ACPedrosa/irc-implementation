#include "server.h"

int main() {
    int socket_fd, connection_fd;
    struct sockaddr_in client;
    socklen_t client_size = (socklen_t)sizeof(client);

    // Criação do socket
    socket_fd = create_server_socket();
    if (socket_fd < 0) return 0;

    // Bind
    bind_socket(socket_fd);

    // Escuta
    listen_for_connections(socket_fd);

    while (1) {
        connection_fd = accept(socket_fd, (struct sockaddr*)&client, &client_size);
        if (connection_fd < 0) {7
            perror("Erro ao aceitar conexão");
            continue;
        }
        printf("Recebi uma mensagem\n");
        handle_client(connection_fd, client);
    }

    close_server_socket(socket_fd);
    return 0;
}