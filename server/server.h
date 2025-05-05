#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
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

pthread_mutex_t mutex_clientes = PTHREAD_MUTEX_INITIALIZER;

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
//cria e configura o socket do servidor
int create_server_socket();

//Associa o socket do servidor a um endereço IP e porta
void bind_socket(int socket_fd);

//Modo escuta do socket do servidor para aguardar conexão do cliente
void listen_for_connections(int socket_fd);

//Aceita e trata uma nova conexão do cliente
void handle_client(int connection_fd, struct sockaddr_in client);

//fecha o socket do servidor
void close_server_socket(int socket_fd);

//Execura thread para lidar com as comunicações com o cliente
void* handle_client_thread(void* arg);

//Execurta uma thread para lidar com as conexões do cliente
void* thread_aceita_conexoes(void* arg);

//Funções para processamento de cliente
//Pega o comando da mensagem enviada pelo cliente
char* get_comando(const char* mensagem);

//Pega a mensagem recebida
char* get_mensagem(const char* mensagem);

//envia uma mensagem para todos os clientes
void enviar_message(int connection_fd, Cliente *clientes, int max_clients, char *message, ssize_t message_size);


// Funções para gerenciamento de clientes
//Verifica o nome de um cliente, se o nome estiver sendo utilizado por um cliente o cliente recbe um NACK, caso não um ACK
char* validar_nome(Cliente *clientes_conectados, const char *nome, const char *ip, int connection_fd);

//Pega o nome do cliente
const char* get_nome_por_fd(int connection_fd);

//Funções para desconexão com o client
//desconecta cliente, removendo-o da ista de clientes conectados
void desconectar_cliente(Cliente *cliente);

//verifica se o cliente está inativo com um intervalo de 60s
void verificar_inatividade();

//Função executada em uma thread para verificar essa inatividade
void* monitorar_inatividade(void* arg);


#endif
