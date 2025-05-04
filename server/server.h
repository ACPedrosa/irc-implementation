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
#include <pthread.h>

#define BUFFER_SIZE 512
#define MAX_CLIENTES 10
#define INATIVIDADE_TIMEOUT 60

// Estrutura para armazenar dados de conexão
typedef struct {
    int connection_fd;
    struct sockaddr_in client;
} ClientData;

// Estrutura para armazenar os dados do cliente 
typedef struct {
    char nome[50];
    char ip[INET_ADDRSTRLEN];
    time_t ultimo_uso;
    ClientData connection_data;  
} Cliente;


// Lista de clientes conectados
extern Cliente clientes_conectados[MAX_CLIENTES]; 
extern int total_clientes; 

// Funções principais do servidor
int create_server_socket();
void bind_socket(int socket_fd);
void listen_for_connections(int socket_fd);
void handle_client(int connection_fd, struct sockaddr_in client);
//void close_server_socket(int socket_fd);
void* handle_client_thread(void* arg);

//processamento de texto
char* get_comando(const char* mensagem);
char* get_mensagem(const char* mensagem);


// Gerenciamento de cliente
char* validar_nome(Cliente *clientes_conectados, const char *nome, const char *ip);

//para desconexão com o client
void desconectar_cliente(Cliente *cliente);
void verificar_inatividade(Cliente *cliente);
void* monitorar_inatividade(void* arg);

#endif
