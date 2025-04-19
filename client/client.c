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
    
    return socket_fd;
}


void communicate_with_server(int socket_fd) {
    // Envia mensagem "PING" para o servidor
    send(socket_fd, "PING", 5, 0);
    printf("Enviei uma mensagem de PING\n");

    // Recebe a resposta do servidor
    char reply[10];
    recv(socket_fd, reply, 10, 0);
    printf("Recebi resposta do servidor: %s\n", reply);
}


