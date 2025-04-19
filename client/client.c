/*
    Cliente - cria socket e se conecta ao servidor
*/

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


int main(){
    int socket_fd; //guarda o id do socket
    struct sockaddr_in target;

    //Criação de socket - IPv4, TCP. 0(flags)
    socket_fd = socket (AF_INET, SOCK_STREAM, 0);
    printf("Socket Criado\n");

    target.sin_family = AF_INET;
    target.sin_port = htons(3001);
    inet_aton("127.0.0.1", &(target.sin_addr));

    printf("Tentando conectar\n");
    if(connect(socket_fd, (struct sockaddr*)&target, sizeof(target)) !=0){
        printf("Problemas ao conectar\n");
        return 0;
    }
     printf("Connectado ao servdior\n");

    send(socket_fd, "PING", 5, 0);
    printf("Escrevi uma mensagem de PING\n");

    char reply[10];
    recv(socket_fd, reply, 10, 0);
    close(socket_fd);
;}
