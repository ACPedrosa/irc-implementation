#include "client.h"

int create_connection(const char *server_ip, int port) {
    int socket_fd;
    struct sockaddr_in target;

    // Criação do socket - IPv4, TCP
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("Erro ao criar o socket");
        return -1;
    }
    printf("Socket criado\n");

    target.sin_family = AF_INET;
    target.sin_port = htons(port);
    inet_aton(server_ip, &(target.sin_addr));

    printf("Tentando conectar ao servidor %s na porta %d...\n", server_ip, port);
    if (connect(socket_fd, (struct sockaddr *)&target, sizeof(target)) != 0) {
        perror("Problemas ao conectar");
        return -1;
    }
    printf("Conectado ao servidor\n");

    printf("Para começar a conversar, por favor insira seu nome\n<NOME> ");

    char nome[50];
    fgets(nome, sizeof(nome), stdin);
    nome[strcspn(nome, "\n")] = '\0';

    char nome_msg[64];
    snprintf(nome_msg, sizeof(nome_msg), "<NOME> %s", nome);
    send(socket_fd, nome_msg, strlen(nome_msg), 0);

    // aguardar ACK/NACK
    char buffer[512];
    ssize_t bytes_received = recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        if (strcmp(buffer, "NACK") == 0) {
            printf("NACK\n");
            //close(socket_fd);
            exit(1);
        } else {
            printf("ACK\n");
        }
    }
   
    return socket_fd;
}

void communicate_with_server(int socket_fd) {

    ssize_t bytes_sent;
    char message[512]; 

    while (1) {
        if (fgets(message, sizeof(message), stdin) != NULL) {
            size_t len = strlen(message);
            if (len > 0 && message[len - 1] == '\n') {
                message[len - 1] = '\0';  
            }

            if (strcmp(message, "<SAIR>") == 0) {
                printf("Desconectando...\n");
                break;
            }            

            bytes_sent = send(socket_fd, message, strlen(message), 0);
            if (bytes_sent < 0) {
                perror("Erro ao enviar mensagem");
                close(socket_fd);
                break;
            }

            char buffer[512];
            ssize_t bytes_received = recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
            if (bytes_received < 0) {
                perror("Erro ao receber dados do servidor");
                close(socket_fd);
                break;
            }

            buffer[bytes_received] = '\0';
            printf("Servidor respondeu: %s\n", buffer);

        } else {
            printf("Erro ao ler a entrada do usuário.\n");
            close(socket_fd);
            break;
        }
    }

    close(socket_fd);
}




