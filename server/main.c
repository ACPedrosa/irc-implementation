#include "server.h"

// Declaração das variáveis globais (extern)
extern int socket_fd;
extern pthread_t thread_conexao;
extern pthread_t thread_inatividade;
extern volatile sig_atomic_t servidor_ativo;

void encerrar_servidor(int sinal) {
    printf("\nEncerrando o servidor...\n");
    servidor_ativo = 0;

     if (thread_inatividade != 0) {
        pthread_cancel(thread_inatividade);
        pthread_join(thread_inatividade, NULL);
        thread_inatividade = 0;
    }

    if (thread_conexao != 0) {
        pthread_cancel(thread_conexao);
        pthread_join(thread_conexao, NULL);
        thread_conexao = 0;
    }

    // Desconecta todos os clientes
    for (int i = 0; i < total_clientes; i++) {
        desconectar_cliente(&clientes_conectados[i]);
    }

    close_server_socket(socket_fd);
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

    // Cria a thread para monitorar a inatividade
    if (pthread_create(&thread_inatividade, NULL, monitorar_inatividade, NULL) != 0) {
        perror("Erro ao criar thread de inatividade");
        close_server_socket(socket_fd);
        pthread_cancel(thread_conexao); // Cancela a thread de conexões
        pthread_join(thread_conexao, NULL);
        return 1;
    }

    // Espera a thread de conexão terminar
    pthread_join(thread_conexao, NULL);
    pthread_join(thread_inatividade, NULL); // Espera a thread de inatividade terminar

    // Fecha o socket do servidor
    close_server_socket(socket_fd);
    return 0;
}
