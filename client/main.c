#include "client.h"

int socket_fd; // Variável global para o socket

int main() {
    socket_fd = create_connection("127.0.0.1", 3001);
    if (socket_fd < 0) {
        return 1; // Retorna 1 para indicar erro
    }

    pthread_t receive_thread;
    if (pthread_create(&receive_thread, NULL, receive_messages, NULL) != 0) {
        perror("Erro ao criar thread de recebimento");
        close(socket_fd);
        return 1;
    }

    communicate_with_server(socket_fd);

    pthread_join(receive_thread, NULL); // Espera a thread terminar
    close(socket_fd);
    return 0;
}
