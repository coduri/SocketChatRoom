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

#define PORT 3334
#define LIMIT 10


// Definizione della struttura che contiene i dati di un singolo client
    typedef struct{
        int sockfd;
        char nome[MAX_SENDER_NAME_LENGTH];
        struct sockaddr_in indirizzo;
    } client_t;


// Prototipi funzioni
    void handleClient(void*);
    void forward(messageProtocol, int);   // per inoltrare/inviare a tutti gli altri client un messaggio <=> necessaria coda di client



// Variabili globali per la gestione della coda di client
    pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;      // mutex per lavorare su una struttura comune a tutti i thread
    client_t *clients[LIMIT];                                       // coda di client_t


// Funzioni per la gestione della coda
    void addQueue(client_t *newClient){
        pthread_mutex_lock(&clients_mutex);

        for(int i=0; i < LIMIT; i++){
            if(clients[i] == NULL){
                clients[i] = newClient;
                break;
            }
        }

        pthread_mutex_unlock(&clients_mutex);
    }

    void removeQueue(int sd){
        pthread_mutex_lock(&clients_mutex);

        for(int i=0; i < LIMIT; i++){
            if(clients[i] != NULL && clients[i]->sockfd == sd){
                clients[i] = NULL;
                break;
            }
        }

        pthread_mutex_unlock(&clients_mutex);
    }




int main() {
    int sd;
    struct sockaddr_in server_add, client_add;
    pthread_t tid;

    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0){
        printErrorStatus("socket creation");
        return -1;
    }
    printOKStatus("socket creation");

    server_add.sin_family = AF_INET;
    server_add.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_add.sin_port = htons(PORT);

    if (bind(sd, (struct sockaddr *) &server_add, sizeof(server_add)) < 0){
        printErrorStatus("binding");
        return -1;
    }
    printOKStatus("binding");
    

    if (listen(sd, LIMIT) < 0){
        printRed("listening");
        return -1;
    }
    printOKStatus("listening");


    while (1) {
        socklen_t sizeCli = sizeof(client_add);
        int clientdescriptor = accept(sd, (struct sockaddr*) &client_add, &sizeCli);

        // Creo spazio nella memoria dinamica e setto le informazioni del nuovo client (tranne nome)
        client_t *cli = (client_t *) malloc(sizeof(client_t));
        cli->indirizzo = client_add;
        cli->sockfd = clientdescriptor;

        // Aggiungo client alla coda e creo thread che lo gestisce
        addQueue(cli);
        pthread_create(&tid, NULL, (void *) handleClient, (void*) cli);
    }

    close(sd);
    return 0;
}


void handleClient(void* arg){
    client_t *thisClient = (client_t *) arg;

    while(1){
        messageProtocol receivedMessage;
        ssize_t bytercv = recv(thisClient->sockfd, &receivedMessage, SIZE_OF_MESSAGE_PROTOCOL, 0);

        if(bytercv <= 0){
            receivedMessage.typeMessage = MESSAGE_TYPE_LEAVE;
            strcpy(receivedMessage.sender, thisClient->nome);

            printUserLeft(thisClient->nome);        // visualizzo l'uscita del client sul server
            forward(receivedMessage, thisClient->sockfd);              // faccio visualizzare l'uscita del client agli altri client
            break;
        }

        else if(receivedMessage.typeMessage == MESSAGE_TYPE_JOIN){
            // Ricevo e setto nella struttura del client il nome
            strcpy(thisClient->nome, receivedMessage.sender);

            printUserJoin(thisClient->nome);        // visualizzo l'ingresso del client sul server
            forward(receivedMessage, thisClient->sockfd);              // faccio visualizzare l'ingresso del client agli altri client
        }

        else if(receivedMessage.typeMessage == MESSAGE_TYPE_SEND){
            printf("%s (%d): %s\n", thisClient->nome, thisClient->sockfd, receivedMessage.message);         // visualizzo messaggio sul server
            forward(receivedMessage, thisClient->sockfd);                                                       // faccio visualizzare il messaggio agli altri client
        }


    }

    // Quando client si è disconnesso (byterecv≤0): chiudo socket, rimuovo dalla coda, libero heap ed elimino thread
    close(thisClient->sockfd);
    removeQueue(thisClient->sockfd);
    free(thisClient);
    pthread_detach(pthread_self());
}



// Devo inoltrare 'msg' a tutti i client tranne che al mittente (nessuno deve modificare la lista dei thread mentre sto inoltrando i messaggi)
void forward(messageProtocol message, int senderSocketDescriptor){
    pthread_mutex_lock(&clients_mutex);

    // Invio messaggio
    for(int i=0; i<LIMIT; i++){
        if(clients[i] != NULL && clients[i]->sockfd != senderSocketDescriptor){   // Tutti i client ricevono il messaggio, tranne colui che l'ha generato
            send(clients[i]->sockfd, &message, SIZE_OF_MESSAGE_PROTOCOL, 0);
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}