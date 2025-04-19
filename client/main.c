#include "client.h"

int main() {
    int socket_fd = create_connection("127.0.0.1", 3001);
    if (socket_fd < 0) {
        return 0;
    }

    communicate_with_server(socket_fd);

    close(socket_fd);
    return 0;
}
