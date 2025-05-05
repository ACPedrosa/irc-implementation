/*
    <NOME> (ok)
    <ACK> ---> aceita o nome do usuário (ok)
    <NACK> ---> não aceita o nome (ok)
    timeout ---> habilitar para desconectar o usuario após 60s de inatividade (ok)
    <NOVO>	notificação de novo cliente conectado 
    <SAIR>	finaliza conexão cliente-servidor (ok)
    <SAIU>	notificação de cliente finalizado (ok)

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
    Para pegar e guardar o comando de cada mensagem enviada para o cliente
*/
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



/*
    Ações para o Comando <NOME> - o servidor deve enviar um ACK caso o nome for aceito e um NACK caso o nome seja repetido
*/
//fazer uma função getNome

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
    clientes_conectados[total_clientes].socket = -1; // Defina o socket posteriormente
    total_clientes++; // Incrementa o contador
    return "ACK"; // nome aceito
}


/*
    Comando <ALL> --> recebe uma mensagem e repassa para outros usuários/clientes
*/
void enviar_message(int connection_fd, int *clients, int max_clients, char *message, ssize_t message_size) {
    for (int i = 0; i < max_clients; i++) {
        if (clients[i] != 0 && clients[i] != connection_fd) { 
            ssize_t bytes_sent = send(clients[i], message, message_size, 0);
            if (bytes_sent < 0) {
                perror("Erro ao enviar mensagem para o cliente");
            }
        }
    }
}


/*
    Operações relacionadas a saida/desconexão dp cliente e servidor
    timeout ---> habilitar para desconectar o usuario após 60s de inatividade
    <SAIR>	finaliza conexão cliente-servidor
    <SAIU>	notificação de cliente finalizado -nome do cliente
*/

void desconectar_cliente(Cliente *cliente) {
    char mensagem[100];
    snprintf(mensagem, sizeof(mensagem), "<SAIU> Cliente %s desconectado", cliente->nome);
    enviar_mensagem(mensagem);
    
    // Fechar a conexão do cliente
    close(cliente->socket);
    printf("Conexão com cliente %s fechada.\n", cliente->nome);
}

// Função para verificar timeout de inatividade
void verificar_inatividade(Cliente *cliente) {
    time_t tempo_atual = time(NULL);
    
    if (difftime(tempo_atual, cliente->ultimo_uso) > INATIVIDADE_TIMEOUT) {
        printf("Cliente %s inativo por %d segundos. Desconectando...\n", cliente->nome, INATIVIDADE_TIMEOUT);
        desconectar_cliente(cliente);
    }
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
    const char* ajuda = /* ... (seu texto de ajuda) ... */;
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

