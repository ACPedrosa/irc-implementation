#include "server.h"

int create_server_socket() {
    //Criação de socket - IPv4, TCP. 0
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("Erro ao criar o socket");
        return -1;
    }
    printf("Socket Criado\n");
    return socket_fd;
}

void bind_socket(int socket_fd) {
    struct sockaddr_in myself;
    myself.sin_family = AF_INET;
    myself.sin_port = htons(3001);
    inet_aton("127.0.0.1", &(myself.sin_addr));

    printf("Tentando abrir porta 3001\n");
    if (bind(socket_fd, (struct sockaddr*)&myself, sizeof(myself)) != 0) {
        perror("Problemas ao abrir a porta");
        close(socket_fd);
        return;
    }
    printf("Porta 3001 aberta\n");
}

void listen_for_connections(int socket_fd) {
    if (listen(socket_fd, 2) != 0) {
        perror("Erro ao ouvir a porta");
        close(socket_fd);
        return;
    }
    printf("Escutando conexões...\n");
}

/*void handle_client(int connection_fd, struct sockaddr_in client) {
    char input_buffer[50];

    recv(connection_fd, input_buffer, 5, 0);
    printf("Mensagem recebida: %s\n", input_buffer);

    // Identificando cliente
    char ip_client[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client.sin_addr), ip_client, INET_ADDRSTRLEN);
    printf("IP do cliente: %s\n", ip_client);

    // Respondendo
    sleep(1);
    printf("Mensagem de retorno\n");
    if (send(connection_fd, "PONG", 5, 0) < 0) {
        printf("Erro: mensagem não enviada\n");
    } else {
        printf("Sucesso ao enviar mensagem\n");
    }
}*/


void handle_client(int connection_fd, struct sockaddr_in client) {
    char input_buffer[BUFFER_SIZE];
    ssize_t bytes_received;

    char ip_client[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client.sin_addr), ip_client, INET_ADDRSTRLEN);
    printf("Novo cliente conectado: %s (%d)\n", ip_client, connection_fd); //depois terá que indicar o nome do novo cliente conectado

    while ((bytes_received = recv(connection_fd, input_buffer, BUFFER_SIZE - 1, 0)) > 0) {
        input_buffer[bytes_received] = '\0'; 

        printf("Servidor recebeu do cliente %s (%d): %s", ip_client, connection_fd, input_buffer);

        // Para enviar mensagem ao cliente
        ssize_t bytes_sent = send(connection_fd, input_buffer, bytes_received, 0);
        if (bytes_sent < 0) {
            perror("Erro ao enviar mensagem de volta para o cliente");
            break;
        }

        if (strchr(input_buffer, '\n') != NULL) {
            break;
        }

        memset(input_buffer, 0, BUFFER_SIZE);
    }

   /* if (bytes_received == 0) {
        printf("Cliente %s (%d) desconectou.\n", ip_client, connection_fd);
    } else if (bytes_received < 0) {
        perror("Erro ao receber mensagem do cliente");
    }*/

    close(connection_fd); // Fecha a conexão com o cliente
}

void close_server_socket(int socket_fd) {
    close(socket_fd);
    printf("Socket fechado\n");
}
