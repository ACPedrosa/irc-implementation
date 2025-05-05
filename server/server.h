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
#include <signal.h>

#define BUFFER_SIZE 512
#define MAX_CLIENTES 10
#define INATIVIDADE_TIMEOUT 60

extern pthread_mutex_t mutex_clientes;

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
int bind_socket(int socket_fd);

//Modo escuta do socket do servidor para aguardar conexão do cliente
int listen_for_connections(int socket_fd);

//Aceita e trata uma nova conexão do cliente
void handle_client(int connection_fd, struct sockaddr_in client);

//fecha o socket do servidor
void close_server_socket(int socket_fd);

//hamadas para desconectar os clientes antes de encerrar o servidor
void encerrar_servidor(int sinal);

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

// Funções auxiliares de tratamento de cliente
//Para processamento de comando e mensagem --> realiza uma ação conforme o recebido
void processar_mensagem(int connection_fd, char* mensagem, const char* ip_client);

//para tratamento de nome 
void handle_nome(int connection_fd, char* nome, const char* ip_client);

//Para tratamento do comando SAIR
void handle_sair(int connection_fd, char* nome_cliente);

//Para tratamento do comando ALL
void handle_all(int connection_fd, char* mensagem);

//Para tratamento do comando Who
void handle_who(int connection_fd);

//Para tratamento do comando Help
void handle_help(int connection_fd);

//Para tratamento do comando whisper
void handle_whisper(int connection_fd, char* nome_destinatario, char* mensagem);



#endif
