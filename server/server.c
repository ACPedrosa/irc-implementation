/*
    Servidor - Socket para ouvir uma porta específica do IP
*/

//bibliotecas
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


int main(){
    int socket_fd, connection_fd; //
    struct sockaddr_in myself, client;
    socklen_t client_size = (socklen_t)sizeof(client);
    char input_buffer[50];


    //Criação de socket - IPv4, TCP. 0
    socket_fd = socket (AF_INET, SOCK_STREAM, 0);
    printf("Socket Criado\n");

    //bind - guardar dados de conexão
    myself.sin_family = AF_INET;
    myself.sin_port = htons(3001);//porta
    inet_aton("127.0.0.1", &(myself.sin_addr)); //converte a string

    printf("Tentando abrir porta 3001\n");
    if(bind(socket_fd, (struct sockaddr*)&myself, sizeof(myself)) !=0){
        printf("Problemas ao abrir a porta\n");
        return 0;
    }
    printf("Porta 3001 aberta\n");

    //escuta da porta - estbelecendo 2 como numero de conexões que podem ficar pendentes
    listen(socket_fd, 2);

    while(1){
        connection_fd = accept(socket_fd, (struct sockaddr*)&client, &client_size);
        printf("Recebi uma mensagem\n");
        recv(connection_fd, input_buffer, 5, 0);
        printf("%s\n", input_buffer);

        /*Identificando cliente*/

        char ip_client[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client.sin_addr),ip_client, INET_ADDRSTRLEN);

       // print()"IP cliente: " << ip_client;

        /*Respondendo*/
        sleep(1);
        printf("Mensagem de retorno\n");
        if(send(connection_fd, "PONG", 5, 0) < 0){
            printf("Error: mensagem não enviada");
        }else{
            printf("Sucesso ao enviar mensagem");
        }
    }

    close(socket_fd);
    return 0;
}
