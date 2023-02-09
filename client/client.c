// Created by Christian Coduri on 26/09/22.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "../libraries/messagePrinting.h"
#include "../libraries/messageProtocol.h"

#define  PORT 3334

void send_handler(int *);
void recv_handler(int *);

char username[MAX_SENDER_NAME_LENGTH] = "";

int main() {
    int sd;
    struct sockaddr_in server_add;
    pthread_t send_thread, recv_thread;

    // Creo e connetto la socket
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0) {
        printErrorStatus("socket creation");
        return -1;
    }

    server_add.sin_family = AF_INET;
    server_add.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_add.sin_port = htons(PORT);

    if (connect(sd, (struct sockaddr *) &server_add, sizeof(server_add)) < 0) {
        printErrorStatus("socket connection");
        return -1;
    }

    // Acquisisco nome utente (mi assicuro che non sia vuoto e che non abbia la newLine)
    while (strcmp(username, "") == 0) {
        printf("Inserisci username utente: ");
        fgets(username, MAX_SENDER_NAME_LENGTH, stdin);

        // Rimuovo il "new line" che fgets acquisisce
        if ((strlen(username) > 0) && (username[strlen(username) - 1] == '\n'))
            username[strlen(username) - 1] = '\0';
    }

    // Invio messaggio di JOIN per avvisare tutti gli altri client
    messageProtocol messageToSend;
    messageToSend.typeMessage = MESSAGE_TYPE_JOIN;

    strcpy(messageToSend.sender, username);
    send(sd, &messageToSend, SIZE_OF_MESSAGE_PROTOCOL, 0);

    // Stampo sul nuovo client messaggio di join
    printUserJoin(username);

    // Creo thread per l'invio di messaggi
    if (pthread_create(&send_thread, NULL, (void *) send_handler, &sd) != 0) {
        printErrorStatus("sender thread creation");
        return -1;
    }

    // Creo thread per la ricezione di messaggi
    if (pthread_create(&recv_thread, NULL, (void *) recv_handler, &sd) != 0) {
        printErrorStatus("receiver thread creation");
        return -1;
    }

    // Resto in attesa del recv_thread, che ritorna solo in caso di connessione col server persa
    pthread_join(recv_thread, NULL);

    close(sd);
    pthread_detach(send_thread);
    pthread_detach(recv_thread);

    return 0;
}


void send_handler(int *sd_pointer) {
    int sd = *sd_pointer;

    // Configuro un template dei messaggi che verranno inviati
    messageProtocol messageToSend;
    messageToSend.typeMessage = MESSAGE_TYPE_SEND;
    strcpy(messageToSend.sender, username);

    while (1) {
        // Stampo un interfaccia del tipo "nome-utente: ..." e resto in attesa di input
        printSendingInterface(username);
        fgets(messageToSend.message, MAX_MESSAGE_LENGTH, stdin);

        // Rimuovo il "new line" che acquisisce fgets
        if ((strlen(messageToSend.message) > 0) && (messageToSend.message[strlen(messageToSend.message) - 1] == '\n'))
            messageToSend.message[strlen(messageToSend.message) - 1] = '\0';

        // Se messaggio non è vuoto lo inivio al server
        if (strcmp(messageToSend.message, "") != 0)
            send(sd, &messageToSend, SIZE_OF_MESSAGE_PROTOCOL, 0);
    }
}

void recv_handler(int *pointer_sd) {
    int sd = *pointer_sd;
    messageProtocol receivedMessage;

    while (1) {
        // Acquisisco nuovo messaggio in arrivo dal server e lo stampo
        ssize_t bytercv = recv(sd, &receivedMessage, SIZE_OF_MESSAGE_PROTOCOL, 0);

        // Rimuovo interfaccia del tipo "nome-utente: ..."
        clearInput();

        if (bytercv <= 0) {
            printErrorStatus("lost connection with the server");
            return;
        }

        else if (receivedMessage.typeMessage == MESSAGE_TYPE_SEND)
            printMessage(receivedMessage.sender, receivedMessage.message);

        else if (receivedMessage.typeMessage == MESSAGE_TYPE_JOIN)
            printUserJoin(receivedMessage.sender);

        else if (receivedMessage.typeMessage == MESSAGE_TYPE_LEAVE)
            printUserLeft(receivedMessage.sender);

        // Ripristino interfaccia del tipo "nome-utente: ..."
        printSendingInterface(username);
        fflush(stdout);
    }
}