#include "server.h"


pthread_mutex_t mutex_clientes = PTHREAD_MUTEX_INITIALIZER;
Cliente clientes_conectados[MAX_CLIENTES];
int total_clientes = 0;
int socket_fd = -1; // Definição das variáveis globais
pthread_t thread_conexao = 0;
pthread_t thread_inatividade = 0;
volatile sig_atomic_t servidor_ativo = 1;

int create_server_socket() {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("Erro ao criar o socket");
        return -1;
    }
    printf("Socket Criado\n");
    return socket_fd;
}

int bind_socket(int socket_fd) {
    struct sockaddr_in myself;
    myself.sin_family = AF_INET;
    myself.sin_port = htons(3001);
    myself.sin_addr.s_addr = INADDR_ANY;

    printf("Tentando abrir porta 3001\n");
    if (bind(socket_fd, (struct sockaddr*)&myself, sizeof(myself)) != 0) {
        perror("Problemas ao abrir a porta");
        close(socket_fd);
        return -1;
    }
    printf("Porta 3001 aberta\n");
    return 0;
}

int listen_for_connections(int socket_fd) {
    if (listen(socket_fd, MAX_CLIENTES) != 0) {
        perror("Erro ao ouvir a porta");
        close(socket_fd);
        return -1;
    }
    printf("Escutando conexões...\n");
    return 0;
}

void handle_client(int connection_fd, struct sockaddr_in client) {
    char input_buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    char ip_client[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client.sin_addr), ip_client, INET_ADDRSTRLEN);
    printf("Novo cliente conectado: %s (%d)\n", ip_client, connection_fd);
   
    for (int i = 0; i < total_clientes; i++) {
        if (clientes_conectados[i].connection_data.connection_fd == connection_fd) {
            clientes_conectados[i].ultimo_uso = time(NULL);
            break;
        }
    }
   
    while (servidor_ativo && (bytes_received = recv(connection_fd, input_buffer, BUFFER_SIZE - 1, 0)) > 0) {
        input_buffer[bytes_received] = '\0';
        printf("Servidor recebeu do cliente %s (%d): %s\n", ip_client, connection_fd, input_buffer);
   
        for (int i = 0; i < total_clientes; i++) {
            if (clientes_conectados[i].connection_data.connection_fd == connection_fd) {
                clientes_conectados[i].ultimo_uso = time(NULL);
                break;
            }
        }
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
        handle_sair(connection_fd);
    } else if (strcmp(comando, "ALL") == 0) {
        handle_all(connection_fd, mensagem_texto);
    } else if (strcmp(comando, "WHO") == 0) {
        handle_who(connection_fd);
    } else if (strcmp(comando, "HELP") == 0) {
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

void handle_sair(int connection_fd) {
    // Busca o cliente pelo ID da conexão
    Cliente cliente;
    for (int i = 0; i < total_clientes; i++) {
        if (clientes_conectados[i].connection_data.connection_fd == connection_fd) {
            cliente = clientes_conectados[i];
            break;
        }
    }
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

void* handle_client_thread(void* arg) {
    ClientData* data = (ClientData*)arg;
    handle_client(data->connection_fd, data->client);
    free(data);
    return NULL;
}

void* thread_aceita_conexoes(void* arg) {
    int socket_fd = *(int*)arg;
    struct sockaddr_in client;
    socklen_t client_size = sizeof(client);
    int connection_fd;

    while (servidor_ativo) { // Usar a flag para controlar o loop
        connection_fd = accept(socket_fd, (struct sockaddr*)&client, &client_size);
        if (connection_fd < 0) {
            if (servidor_ativo) {
                perror("Erro ao aceitar conexão");
            }
            break; // Encerra o loop se o servidor estiver encerrando
        }

        ClientData* client_data = (ClientData*)malloc(sizeof(ClientData));
        if (client_data == NULL) {
            perror("Erro ao alocar memória para cliente");
            close(connection_fd);
            continue;
        }

        client_data->connection_fd = connection_fd;
        client_data->client = client;

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client_thread, (void*)client_data) != 0) {
            perror("Erro ao criar thread para cliente");
            free(client_data);
            close(connection_fd);
        } else {
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
    pthread_mutex_lock(&mutex_clientes);

    char nome_cliente[50];
    strcpy(nome_cliente, cliente->nome); // Copia o nome do cliente

    close(cliente->connection_data.connection_fd); // Fecha o socket primeiro
    printf("Conexão com cliente %s fechada.\n", nome_cliente);

     // Envia mensagem de saída para os outros clientes, se houverem outros clientes conectados.
    if (total_clientes > 0) {
        char mensagem[100];
        snprintf(mensagem, sizeof(mensagem), "<SAIU> Cliente %s desconectou", nome_cliente);
        // Itera sobre todos os clientes conectados, enviando a mensagem para os outros, e não para o que está desconectando.
        for (int i = 0; i < total_clientes; i++) {
            if (clientes_conectados[i].connection_data.connection_fd != cliente->connection_data.connection_fd) {
                 send(clientes_conectados[i].connection_data.connection_fd, mensagem, strlen(mensagem), 0);
            }
        }
    }

    for (int i = 0; i < total_clientes; i++) {
        if (clientes_conectados[i].connection_data.connection_fd == cliente->connection_data.connection_fd) {
            for (int j = i; j < total_clientes - 1; j++) {
                clientes_conectados[j] = clientes_conectados[j + 1];
            }
            total_clientes--;
            break;
        }
    }

    pthread_mutex_unlock(&mutex_clientes);
}


void verificar_inatividade() {
    time_t tempo_atual = time(NULL);
    for (int i = total_clientes - 1; i >= 0; i--) {
        if (difftime(tempo_atual, clientes_conectados[i].ultimo_uso) > INATIVIDADE_TIMEOUT) {
            printf("Cliente %s inativo por mais de %d segundos. Desconectando...\n", clientes_conectados[i].nome, INATIVIDADE_TIMEOUT);
            desconectar_cliente(&clientes_conectados[i]);
        }
    }
}

void* monitorar_inatividade(void* arg) {
    while (servidor_ativo) {
        sleep(5);
        verificar_inatividade();
    }
    return NULL;
}

char* validar_nome(Cliente *clientes_conectados, const char *nome, const char *ip, int connection_fd) {
    pthread_mutex_lock(&mutex_clientes);

    if (total_clientes >= MAX_CLIENTES) {
        pthread_mutex_unlock(&mutex_clientes);
        send(connection_fd, "NACK", strlen("NACK"), 0); // Envia NACK
        return "NACK";
    }

    for (int i = 0; i < total_clientes; i++) {
        if (strcmp(clientes_conectados[i].nome, nome) == 0) {
            pthread_mutex_unlock(&mutex_clientes);
            send(connection_fd, "NACK", strlen("NACK"), 0); // Envia NACK
            return "NACK";
        }
    }

    strncpy(clientes_conectados[total_clientes].nome, nome, sizeof(clientes_conectados[total_clientes].nome));
    strncpy(clientes_conectados[total_clientes].ip, ip, INET_ADDRSTRLEN);
    clientes_conectados[total_clientes].ultimo_uso = time(NULL);
    clientes_conectados[total_clientes].connection_data.connection_fd = connection_fd;
    total_clientes++;

    pthread_mutex_unlock(&mutex_clientes);

    send(connection_fd, "ACK", strlen("ACK"), 0);
    char mensagem_boas_vindas[200];
    snprintf(mensagem_boas_vindas, sizeof(mensagem_boas_vindas), "<ENTROU> Cliente %s entrou no chat", nome);
    enviar_message(connection_fd, clientes_conectados, total_clientes, mensagem_boas_vindas, strlen(mensagem_boas_vindas));
    return "ACK";
}

void enviar_message(int sender_fd, Cliente *clientes, int max_clients, char *message, ssize_t message_size) {
    pthread_mutex_lock(&mutex_clientes);
    for (int i = 0; i < max_clients; i++) {
        int client_fd = clientes[i].connection_data.connection_fd;
        if (client_fd > 0 && client_fd != sender_fd) {
            const char* sender_name = get_nome_por_fd(sender_fd);
            char formatted_message[BUFFER_SIZE];
            snprintf(formatted_message, sizeof(formatted_message), "%s: %s", sender_name, message);
            ssize_t bytes_sent = send(client_fd, formatted_message, strlen(formatted_message), 0);
            if (bytes_sent < 0) {
                perror("Erro ao enviar mensagem para o cliente");
            }
            clientes[i].ultimo_uso = time(NULL);
        }
    }
    pthread_mutex_unlock(&mutex_clientes);
}

void close_server_socket(int socket_fd) {
    if (socket_fd > 0) {
        if (close(socket_fd) != 0) {
            perror("Erro ao fechar o socket");
        }
        printf("Socket %d fechado.\n", socket_fd);
    }
    socket_fd = -1;
}