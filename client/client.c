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


int main() {
    int sd;

    struct sockaddr_in server_add;
    pthread_t send_thread, recv_thread;

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

    pthread_join(recv_thread, NULL);

    return 0;
}


void send_handler(int *sd_pointer) {
    int sd = *sd_pointer;
    char username[MAX_SENDER_NAME_LENGTH];

    messageProtocol messageToSend;
    messageToSend.typeMessage = MESSAGE_TYPE_JOIN;

    // Acquisisco username utente
    printf("Inserisci username utente: ");
    fgets(username, MAX_SENDER_NAME_LENGTH, stdin);
    // Rimuovo il "new line"
    if ((strlen(username) > 0) && (username[strlen (username) - 1] == '\n'))
        username[strlen (username) - 1] = '\0';

    strcpy(messageToSend.sender, username);
    send(sd, &messageToSend, SIZE_OF_MESSAGE_PROTOCOL, 0);

    printUserJoin(username);

    messageToSend.typeMessage = MESSAGE_TYPE_SEND;

    while (1) {
        fgets(messageToSend.message, MAX_MESSAGE_LENGTH, stdin);
        // Rimuovo il "new line"
        if ((strlen(messageToSend.message) > 0) && (messageToSend.message[strlen (messageToSend.message) - 1] == '\n'))
            messageToSend.message[strlen (messageToSend.message) - 1] = '\0';

        // Se messaggio inviato non è vuoto
        if(strcmp(messageToSend.message, "") != 0)
            send(sd, &messageToSend, SIZE_OF_MESSAGE_PROTOCOL, 0);

    }
}

void recv_handler(int *pointer_sd) {
    int sd = *pointer_sd;
    messageProtocol receivedMessage;

    while (1) {
        ssize_t bytercv = recv(sd, &receivedMessage, SIZE_OF_MESSAGE_PROTOCOL, 0);

        // Messaggio inviato da un client
        if (bytercv <= 0) {
            printErrorStatus("lost connection with the server");
            close(sd);
            pthread_detach(pthread_self());

            return;
        }

        else if (receivedMessage.typeMessage == MESSAGE_TYPE_SEND) {
            printMessage(receivedMessage.sender, receivedMessage.message);
        }

        else if (receivedMessage.typeMessage == MESSAGE_TYPE_JOIN) {
            printUserJoin(receivedMessage.sender);
        }

       // Messaggio dal server: indica che c'è stato l'uscita di un client dal gruppo
        else if (receivedMessage.typeMessage == MESSAGE_TYPE_LEAVE) {
            printUserLeft(receivedMessage.sender);
        }
    }
}