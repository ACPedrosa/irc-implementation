#include <stdio.h>
#include <string.h>
#include "iamgui.h"  

#define MAX_MESSAGE_LEN 512

char message_buffer[MAX_MESSAGE_LEN];  

//função básica
void send_message(void) {
    printf("Mensagem enviada: %s\n", message_buffer);
    memset(message_buffer, 0, MAX_MESSAGE_LEN); 
}

// Função chamada quando um botão é pressionado
void on_send_button_clicked(void) {
    send_message();
}

// Função para atualizar o buffer de mensagens
void on_text_changed(const char *text) {
    strncpy(message_buffer, text, MAX_MESSAGE_LEN);
}


void create_gui(void) {
    IamGui_Init(); 

    // Nome da janela, dimensões
    IamGui_CreateWindow("IRC Chat", 600, 400); 

    // Caixa de exibição de mensagens
    IamGui_CreateTextArea(10, 10, 580, 200, "Messages"); 

    // Caixa de entrada de texto 
    IamGui_CreateTextBox(10, 220, 480, 30, "Digite sua mensagem", on_text_changed);

    // Botão para 
    IamGui_CreateButton(500, 220, 80, 30, "Enviar", on_send_button_clicked);

    // Loop para a interface
    while (IamGui_PollEvents()) {
        //add eventos gui
    }

    IamGui_Shutdown();  // Libera os recursos da GUI
}

int main() {
    create_gui();
    
    return 0;
}
