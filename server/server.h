#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define BUFFER_SIZE 512
#define MAX_CLIENTES 10
#define INATIVIDADE_TIMEOUT 60

typedef struct {
    int socket;
    char nome[50];
    char ip[INET_ADDRSTRLEN];
    time_t ultimo_uso;
} Cliente;

// Funções principais do servidor
int create_server_socket();
void bind_socket(int socket_fd);
void listen_for_connections(int socket_fd);
void handle_client(int connection_fd, struct sockaddr_in client);
void close_server_socket(int socket_fd);

// Gerenciamento de cliente
void desconectar_cliente(Cliente *cliente);
void verificar_inatividade(Cliente *cliente);


#endif
