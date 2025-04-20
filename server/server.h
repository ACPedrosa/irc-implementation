#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 512

int create_server_socket();
void bind_socket(int socket_fd);
void listen_for_connections(int socket_fd);
void handle_client(int connection_fd, struct sockaddr_in client);
void close_server_socket(int socket_fd);

#endif
