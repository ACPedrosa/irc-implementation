/*
    <NOME> (ok)
    <ACK> ---> aceita o nome do usuário (ok)
    <NACK> ---> não aceita o nome (ok)
    timeout ---> habilitar para desconectar o usuario após 60s de inatividade (ok)
    <NOVO>	notificação de novo cliente conectado 
    <SAIR>	finaliza conexão cliente-servidor (ok)
    <SAIU>	notificação de cliente finalizado (ok)

    <DESTINO>		whisper
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

char* validar_nome(UserDictionary *dict, const char *nome, const char *ip){
    for (int i = 0; i < dict->size; i++) {
        if (strcmp(dict->users[i].name, name) == 0 || strcmp(dict->users[i].ip, ip) == 0) {
            return 0; // NACK
        }
    }

    strcpy(dict->users[dict->size].name, name);
    strcpy(dict->users[dict->size].ip, ip);
    dict->size++;
    return 1; //ACK
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

void desconect_client(Cliente *cliente, const char *comando) {
    if (comando != NULL && strcmp(comando, "<SAIR>") == 0) {
        printf("Comando <SAIR> recebido. Desconectando cliente %s...\n", cliente->nome);
        desconectar_cliente(cliente);
    } else {
        verificar_inatividade(cliente);
    }
}
