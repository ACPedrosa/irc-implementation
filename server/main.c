#include "server.h"

int main() {
    int socket_fd, connection_fd;
    struct sockaddr_in client;
    socklen_t client_size = (socklen_t)sizeof(client);

    // Criação do socket
    socket_fd = create_server_socket();
    if (socket_fd < 0) return 0;

    // Bind
    bind_socket(socket_fd);

    // Escuta
    listen_for_connections(socket_fd);

    while (1) {
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

    close_server_socket(socket_fd);
    return 0;
}