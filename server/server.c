#include "server.h"

Cliente clientes_conectados[MAX_CLIENTES];
int total_clientes = 0;

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
    myself.sin_addr.s_addr = INADDR_ANY; //aceita qualquer IP
    //inet_aton("127.0.0.1", &(myself.sin_addr));

    printf("Tentando abrir porta 3001\n");
    if (bind(socket_fd, (struct sockaddr*)&myself, sizeof(myself)) != 0) {
        perror("Problemas ao abrir a porta");
        close(socket_fd);
        return;
    }
    printf("Porta 3001 aberta\n");
}

void listen_for_connections(int socket_fd) {
    if (listen(socket_fd, MAX_CLIENTES) != 0) {
        perror("Erro ao ouvir a porta");
        close(socket_fd);
        return;
    }
    printf("Escutando conexões...\n");
}

void handle_client(int connection_fd, struct sockaddr_in client) {
    char input_buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    char ip_client[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client.sin_addr), ip_client, INET_ADDRSTRLEN);
    printf("Novo cliente conectado: %s (%d)\n", ip_client, connection_fd);

    while ((bytes_received = recv(connection_fd, input_buffer, BUFFER_SIZE - 1, 0)) > 0) {
        input_buffer[bytes_received] = '\0';
        printf("Servidor recebeu do cliente %s (%d): %s\n", ip_client, connection_fd, input_buffer);

        // Obter comando e mensagem
        char *comando = get_comando(input_buffer);
        char *mensagem = get_mensagem(input_buffer);

        if (comando == NULL) {
            printf("Comando inválido.\n");
            continue;
        }

        if (strcmp(comando, "NOME") == 0) {
            char *resposta = validar_nome(clientes_conectados, mensagem, ip_client);
            send(connection_fd, resposta, strlen(resposta), 0);
        } 
        else if (strcmp(comando, "SAIR") == 0) {
            Cliente cliente;
            strcpy(cliente.nome, mensagem); // ou identifique pelo fd
            cliente.connection_data.connection_fd = connection_fd;
            desconectar_cliente(&cliente);
            break;
        } 
        else if (strcmp(comando, "ALL") == 0) {
            enviar_message(connection_fd, clientes_conectados, total_clientes, mensagem, strlen(mensagem));
        } 
        else {
            printf("Comando desconhecido: %s\n", comando);
        }

        free(comando);
        free(mensagem);
        memset(input_buffer, 0, BUFFER_SIZE);
    }

    close(connection_fd); // Fecha a conexão com o cliente
}

// Função para tratar a conexão de um cliente
void* handle_client_thread(void* arg) {
    ClientData* data = (ClientData*)arg;
    handle_client(data->connection_fd, data->client);
    close(data->connection_fd);  // Fecha a conexão do cliente
    free(data);  // Libera a memória alocada para os dados do cliente
    return NULL;
}

char* get_comando(const char* mensagem) {
    if (!mensagem || mensagem[0] != '<') return NULL;

    const char* inicio = mensagem + 1;
    const char* fim = strchr(inicio, '>');

    if (!fim || fim == inicio) return NULL;

    size_t tamanho = fim - inicio;
    char* comando = (char*)malloc(tamanho + 1);
    if (!comando) return NULL;

    strncpy(comando, inicio, tamanho);
    comando[tamanho] = '\0';

    return comando;
}

char* get_mensagem(const char* mensagem) {
    const char* fim = strchr(mensagem, '>');

    if (!fim || *(fim + 1) == '\0') return NULL;

    const char* inicioMsg = fim + 1;
    while (*inicioMsg == ' ') inicioMsg++;

    if (*inicioMsg == '\0') return NULL;

    return strdup(inicioMsg);
}


void desconectar_cliente(Cliente *cliente) {
    char mensagem[100];
    snprintf(mensagem, sizeof(mensagem), "<SAIU> Cliente %s desconectado", cliente->nome);
    enviar_message(cliente->connection_data.connection_fd
        , clientes_conectados, total_clientes, mensagem, strlen(mensagem));
    
    // Fechar a conexão do cliente
    close(cliente->connection_data.connection_fd);
    printf("Conexão com cliente %s fechada.\n", cliente->nome);
    
    // Remover o cliente da lista de clientes conectados
    for (int i = 0; i < total_clientes; i++) {
        if (clientes_conectados[i].connection_data.connection_fd == cliente->connection_data.connection_fd){
            // Movendo todos os outros clientes para "fechar" a posição do cliente desconectado
            for (int j = i; j < total_clientes - 1; j++) {
                clientes_conectados[j] = clientes_conectados[j + 1];
            }
            total_clientes--;
            break;
        }
    }
}


// Função para verificar timeout de inatividade
void verificar_inatividade() {
    time_t tempo_atual = time(NULL);
    
    for (int i = 0; i < total_clientes; i++) {
        if (difftime(tempo_atual, clientes_conectados[i].ultimo_uso) > INATIVIDADE_TIMEOUT) {
            printf("Cliente %s inativo por mais de %d segundos. Desconectando...\n", clientes_conectados[i].nome, INATIVIDADE_TIMEOUT);
            desconectar_cliente(&clientes_conectados[i]);
        }
    }
}

void* monitorar_inatividade(void* arg) {
    while (1) {
        sleep(5);
        verificar_inatividade();
    }
}

char* validar_nome(Cliente *clientes_conectados, const char *nome, const char *ip) {
    for (int i = 0; i < total_clientes; i++) {
        if (strcmp(clientes_conectados[i].nome, nome) == 0 || strcmp(clientes_conectados[i].ip, ip) == 0) {
            return "NACK"; // nome ou IP já existe
        }
    }

    // Adicionando o cliente à lista
    strcpy(clientes_conectados[total_clientes].nome, nome);
    strcpy(clientes_conectados[total_clientes].ip, ip);
    clientes_conectados[total_clientes].ultimo_uso = time(NULL); // Registrar o último uso
    clientes_conectados[total_clientes].connection_data.connection_fd = -1;
    total_clientes++; // Incrementa o contador
    return "ACK"; // nome aceito
}

void enviar_message(int connection_fd, Cliente *clientes, int max_clients, char *message, ssize_t message_size) {
    for (int i = 0; i < max_clients; i++) {
        int client_fd = clientes[i].connection_data.connection_fd;
        if (client_fd != 0 && client_fd != connection_fd) { 
            ssize_t bytes_sent = send(client_fd, message, message_size, 0);
            if (bytes_sent < 0) {
                perror("Erro ao enviar mensagem para o cliente");
            }
        }
    }
}
