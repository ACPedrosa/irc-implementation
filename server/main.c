#include "server.h"

int main() {
    int socket_fd;
    pthread_t thread_conexao;

    // Criação do socket
    socket_fd = create_server_socket();
    if (socket_fd < 0) return 0;

    // Bind
    bind_socket(socket_fd);

    // Escuta
    listen_for_connections(socket_fd);
    
    // Cria uma thread para aceitar conexões
    if (pthread_create(&thread_conexao, NULL, thread_aceita_conexoes, &socket_fd) != 0) {
        perror("Erro ao criar thread de conexões");
        close_server_socket(socket_fd);
        return 1;
    }

    pthread_join(thread_conexao, NULL);

    close_server_socket(socket_fd);
    return 0;
}
