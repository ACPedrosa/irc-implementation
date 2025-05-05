#include "server.h"

int main() {
    int socket_fd;
    pthread_t thread_conexao;

    // Criação do socket
    socket_fd = create_server_socket();
    if (socket_fd < 0) {
        fprintf(stderr, "Erro ao criar socket, encerrando servidor\n");
        return 1;
    }

    // Bind
    if (bind_socket(socket_fd) != 0) {
        fprintf(stderr, "Erro ao fazer bind do socket, encerrando servidor\n");
        close_server_socket(socket_fd);
        return 1;
    }

    // Escuta
    if (listen_for_connections(socket_fd) != 0) {
        fprintf(stderr, "Erro ao escutar na porta, encerrando servidor\n");
        close_server_socket(socket_fd);
        return 1;
    }

    // Cria uma thread para aceitar conexões
    if (pthread_create(&thread_conexao, NULL, thread_aceita_conexoes, &socket_fd) != 0) {
        perror("Erro ao criar thread de conexões");
        close_server_socket(socket_fd);
        return 1;
    }

    // Espera a thread de conexão terminar (ou ser cancelada)
    pthread_join(thread_conexao, NULL);

    close_server_socket(socket_fd);
    return 0;
}
