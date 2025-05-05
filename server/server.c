#include "server.h"

pthread_mutex_t mutex_clientes = PTHREAD_MUTEX_INITIALIZER;
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

        processar_mensagem(connection_fd, input_buffer, ip_client);
        memset(input_buffer, 0, BUFFER_SIZE);
    }
    close(connection_fd);
    printf("Conexão com cliente %s (%d) fechada.\n", ip_client, connection_fd);
}

void processar_mensagem(int connection_fd, char* mensagem, const char* ip_client) {
    char* comando = get_comando(mensagem);
    char* mensagem_texto = get_mensagem(mensagem);

    if (comando == NULL) {
        printf("Comando inválido.\n");
        free(mensagem_texto);
        return;
    }

    if (strcmp(comando, "NOME") == 0) {
        handle_nome(connection_fd, mensagem_texto, ip_client);
    } else if (strcmp(comando, "SAIR") == 0) {
        handle_sair(connection_fd, mensagem_texto);
    } else if (strcmp(comando, "ALL") == 0) {
        handle_all(connection_fd, mensagem_texto);
    } else if (strcmp(comando, "WHO") == 0) {
        handle_who(connection_fd);
    } else if (strcmp(comando, "<HELP>") == 0) {
        handle_help(connection_fd);
    } else {
        handle_whisper(connection_fd, comando, mensagem_texto);
    }

    free(comando);
    free(mensagem_texto);
}

void handle_nome(int connection_fd, char* nome, const char* ip_client) {
    char* resposta = validar_nome(clientes_conectados, nome, ip_client, connection_fd);
    send(connection_fd, resposta, strlen(resposta), 0);
}

void handle_sair(int connection_fd, char* nome_cliente) {
    Cliente cliente;
    strcpy(cliente.nome, nome_cliente);
    cliente.connection_data.connection_fd = connection_fd;
    desconectar_cliente(&cliente);
}

void handle_all(int connection_fd, char* mensagem) {
    enviar_message(connection_fd, clientes_conectados, total_clientes, mensagem, strlen(mensagem));
}

void handle_who(int connection_fd) {
    char resposta[BUFFER_SIZE];
    snprintf(resposta, sizeof(resposta), "Clientes conectados (%d):\n", total_clientes);
    for (int i = 0; i < total_clientes; i++) {
        strncat(resposta, "- ", sizeof(resposta) - strlen(resposta) - 1);
        strncat(resposta, clientes_conectados[i].nome, sizeof(resposta) - strlen(resposta) - 1);
        strncat(resposta, "\n", sizeof(resposta) - strlen(resposta) - 1);
    }
    send(connection_fd, resposta, strlen(resposta), 0);
}

void handle_help(int connection_fd) {
    const char* ajuda =
    "Comandos disponíveis:\n"
    "<ALL>             - Enviar mensagem a todos\n "
    "<SAIR>            - Sair do chat\n"
    "<WHO>             - Ver usuários conectados\n"
    "<HELP>            - Mostrar esta ajuda\n"
    "<nome> <mensagem> - Enviar mensagem privada (whisper)\n";
    send(connection_fd, ajuda, strlen(ajuda), 0);
}

void handle_whisper(int connection_fd, char* nome_destinatario, char* mensagem) {
    int encontrado = 0;
    for (int i = 0; i < total_clientes; i++) {
        if (strcmp(clientes_conectados[i].nome, nome_destinatario) == 0) {
            const char* nome_remetente = get_nome_por_fd(connection_fd);
            char msg_formatada[BUFFER_SIZE];
            snprintf(msg_formatada, sizeof(msg_formatada), "[Whisper de %s]: %s", nome_remetente, mensagem);
            send(clientes_conectados[i].connection_data.connection_fd, msg_formatada, strlen(msg_formatada), 0);
            encontrado = 1;
            break;
        }
    }
    if (!encontrado) {
        char resposta[BUFFER_SIZE];
        snprintf(resposta, sizeof(resposta), "Comando '%s' não reconhecido. Digite <HELP> para ajuda.\n", nome_destinatario);
        send(connection_fd, resposta, strlen(resposta), 0);
    }
}


// Função para tratar a conexão de um cliente
void* handle_client_thread(void* arg) {
    ClientData* data = (ClientData*)arg;
    handle_client(data->connection_fd, data->client);
    close(data->connection_fd);  // Fecha a conexão do cliente
    free(data);  // Libera a memória alocada para os dados do cliente
    return NULL;
}

// Função para aceitar conexões
void* thread_aceita_conexoes(void* arg) {
    int socket_fd = *(int*)arg;
    struct sockaddr_in client;
    socklen_t client_size = sizeof(client);
    int connection_fd;

    while (1) {
        // Aceita a conexão
        connection_fd = accept(socket_fd, (struct sockaddr*)&client, &client_size);
        if (connection_fd < 0) {
            perror("Erro ao aceitar conexão");
            continue;
        }

        // Aloca memória para armazenar os dados do cliente
        ClientData* client_data = (ClientData*)malloc(sizeof(ClientData));
        if (client_data == NULL) {
            perror("Erro ao alocar memória para cliente");
            close(connection_fd);
            continue;
        }

        // Preenche os dados do cliente
        client_data->connection_fd = connection_fd;
        client_data->client = client;

        // Cria uma thread para tratar o cliente
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client_thread, (void*)client_data) != 0) {
            perror("Erro ao criar thread para cliente");
            free(client_data);  // Libera a memória caso a thread não seja criada
            close(connection_fd);
        } else {
            // Detach a thread para que ela seja limpa automaticamente após terminar
            pthread_detach(thread_id);
        }
    }

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

const char* get_nome_por_fd(int connection_fd) {
    for (int i = 0; i < total_clientes; i++) {
        if (clientes_conectados[i].connection_data.connection_fd == connection_fd) {
            return clientes_conectados[i].nome;
        }
    }
    return "Desconhecido";
}



void desconectar_cliente(Cliente *cliente) {
    pthread_mutex_lock(&mutex_clientes);  // Bloqueia o mutex antes de modificar os dados

    char mensagem[100];
    snprintf(mensagem, sizeof(mensagem), "<SAIU> Cliente %s desconectado", cliente->nome);
    enviar_message(cliente->connection_data.connection_fd
        , clientes_conectados, total_clientes, mensagem, strlen(mensagem));

    // Fechar a conexão do cliente
    close(cliente->connection_data.connection_fd);
    printf("Conexão com cliente %s fechada.\n", cliente->nome);

    for (int i = 0; i < total_clientes; i++) {
        if (clientes_conectados[i].connection_data.connection_fd == cliente->connection_data.connection_fd) {
            for (int j = i; j < total_clientes - 1; j++) {
                clientes_conectados[j] = clientes_conectados[j + 1];
            }
            total_clientes--;
            break;
        }
    }

    pthread_mutex_unlock(&mutex_clientes);  // Libera o mutex após modificar os dados
}



// Função para verificar timeout de inatividade
void verificar_inatividade() {
    time_t tempo_atual = time(NULL);
    
    for (int i = total_clientes -1; i >= 0; i--) {
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

char* validar_nome(Cliente *clientes_conectados, const char *nome, const char *ip, int connection_fd) {
    pthread_mutex_lock(&mutex_clientes);  // Bloqueia o mutex antes de modificar os dados

    // Verifica se já existe um cliente com o mesmo nome ou IP
    for (int i = 0; i < total_clientes; i++) {
        if (strcmp(clientes_conectados[i].nome, nome) == 0) { //|| strcmp(clientes_conectados[i].ip, ip) == 0
            pthread_mutex_unlock(&mutex_clientes);  // Libera o mutex antes de retornar
            return "NACK"; 
        }
    }

    // Adiciona o cliente à lista de clientes conectados
    strncpy(clientes_conectados[total_clientes].nome, nome, sizeof(clientes_conectados[total_clientes].nome));
    strncpy(clientes_conectados[total_clientes].ip, ip, INET_ADDRSTRLEN);
    clientes_conectados[total_clientes].ultimo_uso = time(NULL); // Registra o último uso
    clientes_conectados[total_clientes].connection_data.connection_fd = connection_fd; // Registra a conexão
    total_clientes++; // Incrementa o contador

    pthread_mutex_unlock(&mutex_clientes);  // Libera o mutex após modificar os dados

    return "ACK"; // Nome aceito
}



void enviar_message(int connection_fd, Cliente *clientes, int max_clients, char *message, ssize_t message_size) {
    pthread_mutex_lock(&mutex_clientes);  // Bloqueia o mutex antes de modificar os dados

    for (int i = 0; i < max_clients; i++) {
        int client_fd = clientes[i].connection_data.connection_fd;
        if (client_fd != 0 && client_fd != connection_fd) {
            ssize_t bytes_sent = send(client_fd, message, message_size, 0);
            if (bytes_sent < 0) {
                perror("Erro ao enviar mensagem para o cliente");
            }
        }
    }

    pthread_mutex_unlock(&mutex_clientes);  // Libera o mutex após modificar os dados
}

void close_server_socket(int socket_fd) {
    if (socket_fd > 0) { 
        if (close(socket_fd) != 0) {
            perror("Erro ao fechar o socket");
        }
        printf("Socket %d fechado.\n", socket_fd);
    }
}