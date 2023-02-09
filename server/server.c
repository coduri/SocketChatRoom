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
typedef struct {
    int sockfd;
    struct sockaddr_in indirizzo;
} client_t;


// Prototipi delle funzioni di gestione client e forwarding
void handleClient(void *);              // Funzione di gestione client data in pasto al thread
void forward(messageProtocol, int);     // Funzione per inoltrare a tutti gli altri client un messaggio <=> necessaria coda di client


// Variabili globali per la gestione della coda di client
client_t *clients[LIMIT];                                   // Coda di client_t (puntatori a struttura perché allocati dinamicamente)
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;  // Mutex per lavorare sulla struttura coda, comune a tutti i thread


// Funzione per aggiungere un thread alla coda
void addQueue(client_t *newClient) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < LIMIT; i++) {
        if (clients[i] == NULL) {
            clients[i] = newClient;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Funzione per rimuovere un thread dalla coda
void removeQueue(int sd) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < LIMIT; i++) {
        if (clients[i] != NULL && clients[i]->sockfd == sd) {
            clients[i] = NULL;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}


int main() {
    int sd;
    struct sockaddr_in server_add, client_add;
    socklen_t sizeCli = sizeof(client_add);
    pthread_t tid;

    // Creo e metto in ascolto la socket
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0) {
        printErrorStatus("socket creation");
        return -1;
    }
    printOKStatus("socket creation");

    server_add.sin_family = AF_INET;
    server_add.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_add.sin_port = htons(PORT);

    if (bind(sd, (struct sockaddr *) &server_add, sizeof(server_add)) < 0) {
        printErrorStatus("binding");
        return -1;
    }
    printOKStatus("binding");

    if (listen(sd, LIMIT) < 0) {
        printErrorStatus("listening");
        return -1;
    }
    printOKStatus("listening");


    while (1) {
        // Accetto connessione e salvo socket descriptor di tale client
        int clientdescriptor = accept(sd, (struct sockaddr *) &client_add, &sizeCli);

        // Creo spazio nella memoria dinamica e setto le informazioni del nuovo client
        client_t *cli = (client_t *) malloc(sizeof(client_t));
        cli->indirizzo = client_add;
        cli->sockfd = clientdescriptor;

        // Aggiungo il client alla coda e creo un thread che lo gestisce
        addQueue(cli);
        pthread_create(&tid, NULL, (void *) handleClient, (void *) cli);
    }

    // close(sd); -- mai eseguito
    return 0;
}


// Funzione eseguita da ogni thread che gestisce un certo client: riceve messaggio e inoltra a tutti gli altri client.
void handleClient(void *arg) {
    client_t *thisClient = (client_t *) arg;
    messageProtocol receivedMessage;

    while (1) {
        ssize_t bytercv = recv(thisClient->sockfd, &receivedMessage, SIZE_OF_MESSAGE_PROTOCOL, 0);

        if (bytercv <= 0) {
            receivedMessage.typeMessage = MESSAGE_TYPE_LEAVE;
            printUserLeft(receivedMessage.sender);
            forward(receivedMessage, thisClient->sockfd);
            break;
        }

        else if (receivedMessage.typeMessage == MESSAGE_TYPE_JOIN)
            printUserJoin(receivedMessage.sender);

        else if (receivedMessage.typeMessage == MESSAGE_TYPE_SEND)
            printf("%s (%d): %s\n", receivedMessage.sender, thisClient->sockfd, receivedMessage.message);

        forward(receivedMessage, thisClient->sockfd);
    }

    // Quando client si disconnette (byterecv ≤ 0): chiudo socket, rimuovo client dalla coda, libero memoria dinamica ed elimino thread
    close(thisClient->sockfd);
    removeQueue(thisClient->sockfd);
    free(thisClient);
    pthread_detach(pthread_self());
}


// Funzione di appoggio che si occupa di inoltrare i messaggi a tutti i client tranne a colui che l'ha generato
void forward(messageProtocol message, int senderSocketDescriptor) {
    pthread_mutex_lock(&clients_mutex);     // Acquisico lock, nessuno deve modificare la lista dei thread mentre sto inoltrando i messaggi.

    // Invio messaggio
    for (int i = 0; i < LIMIT; i++)
        if (clients[i] != NULL && clients[i]->sockfd != senderSocketDescriptor)
            send(clients[i]->sockfd, &message, SIZE_OF_MESSAGE_PROTOCOL, 0);

    pthread_mutex_unlock(&clients_mutex);   // Rilascio lock
}