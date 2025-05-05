#include "server.h"

int socket_fd = -1; 
pthread_t thread_conexao = 0;

void encerrar_servidor(int sinal) {
    printf("\nEncerrando o servidor...\n");
     for (int i = 0; i < total_clientes; i++) {
        desconectar_cliente(&clientes_conectados[i]);
    }
    if (socket_fd != -1) {
        close_server_socket(socket_fd);
    }
    if (thread_conexao != 0) {
        pthread_cancel(thread_conexao);
        pthread_join(thread_conexao, NULL);
    }
    exit(0);
}

int main() {
    signal(SIGINT, encerrar_servidor);
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

    pthread_join(thread_conexao, NULL);
    close_server_socket(socket_fd);
    return 0;
}

