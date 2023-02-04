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
#include "messagePrinting.h"

#define MAXLEN 1000
#define  PORT 3334


void send_handler(int*);
void recv_handler(int*);


int main() {
    int sd;
    char nome[30];
    struct sockaddr_in server_add;
    pthread_t send_thread, recv_thread;

    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0){
        printRed("Errore creazione socket!");
        return -1;
    }

    server_add.sin_family = AF_INET;
    server_add.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_add.sin_port = htons(PORT);

    if(connect(sd, (struct sockaddr*) &server_add, sizeof(server_add)) < 0){
        printRed( "Errore connessione al server!");
        return -1;
    }

    // Acquisisco nome utente e lo invio al server
    printf("Inserisci nome utente: ");
    scanf("%s", nome);

    send(sd, nome, 30, 0);
 
    printGreen("\n=== Ti sei unito alla chat ===\n");

    // Creo thread per l'invio di messaggi
    if(pthread_create(&send_thread, NULL, (void *) send_handler, &sd) != 0){
        printRed("Errore creazione send-thread!");
        return -1;
    }

    // Creo thread per la ricezione di messaggi
    if(pthread_create(&recv_thread, NULL, (void *) recv_handler, &sd) != 0){
        printRed("Errore creazione recv-thread!");
        return -1;
    }

    while(1);

    close(sd);
    return 0;
}


void send_handler(int* pointer_sd) {
    int sd = *pointer_sd;
    char sendbuff[MAXLEN];

    memset(sendbuff, 0, MAXLEN);

    while(1){
        printf(COLOR_RESET);

        fgets(sendbuff, MAXLEN, stdin);
        send(sd, sendbuff, MAXLEN, 0);
    }
}

void recv_handler(int* pointer_sd) {
    int sd = *pointer_sd;
    char recvbuff[MAXLEN], nomeClient[30];
    int option;

    memset(recvbuff, 0, MAXLEN);

    while(1){
        // Ricevo opzione che mi indica il tipo di messaggio (da server o da altro client)
        recv(sd, &option, sizeof(int), 0);
        int msgType = ntohl(option);

        // Messaggio inviato da un client
        if(msgType == 1){
            recv(sd, nomeClient, 30, 0);    // ricevo nome del mittente
            recv(sd, recvbuff, MAXLEN, 0);  // ricevo messaggio

            printMessage(nomeClient, recvbuff);
        }

        // Messaggio dal server: indica che c'è stato l'ingresso di un nuovo client nel gruppo
        else if(msgType == 2){
            recv(sd, nomeClient, 30, 0);  // ricevo nome del mittente

            printUserJoin(nomeClient);
        }

        // Messaggio dal server: indica che c'è stato l'uscita di un client dal gruppo
        else if(msgType == 3){
            recv(sd, nomeClient, 30, 0);  // ricevo nome del mittente

            printUserLeft(nomeClient);
        }    
    }
}