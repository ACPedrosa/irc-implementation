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
    ssize_t bytes_sent;
    char message[512]; //512 é o padrão do IRC

    printf("MSG: "); //depois modificar para o username do user
    if (fgets(message, sizeof(message), stdin) != NULL) {
        size_t len = strlen(message);
        if (len > 0 && message[len - 1] == '\n') {
            message[len - 1] = '\0';
        }

        bytes_sent = send(socket_fd, message, strlen(message), 0);
        if (bytes_sent < 0) {
            perror("Erro ao enviar mensagem");
            close(socket_fd);
        } 
    } else {
        printf("Erro ao ler a entrada do usuário.\n"); //depois fazer um tratamento melhor, isso deria para o server
        close(socket_fd);
    }
}



