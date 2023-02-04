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
#define PORT 3334
#define LIMIT 10


// Definizione della struttura che contiene i dati di un singolo client
    typedef struct{
        int sockfd;
        char nome[30];
        struct sockaddr_in indirizzo;
    } client_t;


// Prototipi funzioni
    void handleClient(void*);
    void forward(int, char*, client_t*);   // per inoltrare/inviare a tutti gli altri client un messaggio <=> necessaria coda di client



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
    struct sockaddr_in server_add;
    struct sockaddr_in client_add;
    pthread_t tid;

    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0){
        printRed("Errore creazione socket!");
        return -1;
    }

    printGreen("OK: "); printf("socket()\n");
    

    server_add.sin_family = AF_INET;
    server_add.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_add.sin_port = htons(PORT);
    

    if (bind(sd, (struct sockaddr *) &server_add, sizeof(server_add)) < 0){
        printRed("Errore bind()!");
        return -1;
    }

    printGreen("OK: "); printf("bind()\n");
    

    if (listen(sd, LIMIT) < 0){
        printRed("Errore listen()!");
        return -1;
    }

    printGreen("OK: "); printf("listen()\n\n");


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
    char recvbuff[MAXLEN];
    client_t *cli = (client_t *) arg;

    memset(recvbuff, 0, MAXLEN);

    // Ricevo e setto nella struttura del client il nome
    recv(cli->sockfd, recvbuff, 30, 0);
    strcpy(cli->nome, recvbuff);

    printUserJoin(cli->nome);     // visualizzo l'ingresso del nuovo client sul server
    forward(2, NULL, cli);                                                              // faccio visualizzare l'ingresso del nuovo client agli altri client

    while(1){
        int bytercv = recv(cli->sockfd, recvbuff, MAXLEN, 0);

        if(bytercv > 0){
            printf("%s (%d): %s", cli->nome, cli->sockfd, recvbuff);                    // visualizzo messaggio sul server
            forward(1, recvbuff, cli);                                                  // faccio visualizzare il messaggio agli altri client
        }

        else if(bytercv <= 0){
            printUserLeft(cli->nome);     // visualizzo l'uscita del client sul server
            forward(3, NULL, cli);                                                      // faccio visualizzare l'uscita del client agli altri client
            break;
        }
    }

    // chiudo socket, rimuovo dalla coda, libero heap ed elimino thread
    close(cli->sockfd);
    removeQueue(cli->sockfd);
    free(cli);
    pthread_detach(pthread_self());
}



// Devo inoltrare 'msg' a tutti i client tranne che al mittente (nessuno deve modificare la lista dei thread mentre sto inoltrando i messaggi)
void forward(int msgType, char* msg, client_t * mittente){
    pthread_mutex_lock(&clients_mutex);     

    // Invio messaggio
    for(int i=0; i<LIMIT; i++){
        if(clients[i] != NULL && clients[i]->sockfd != mittente->sockfd){   // Client che deve ricevere il messaggio

            // Invio tipo di messaggio
            int converted_number = htonl(msgType);
            send(clients[i]->sockfd, &converted_number, sizeof(converted_number), 0);


            // Messaggio inviato da un client => server fa un semplice inoltro
            if(msgType == 1){
                send(clients[i]->sockfd, mittente->nome, 30, 0);    // invio nome mittente
                send(clients[i]->sockfd, msg, MAXLEN, 0);           // invio messaggio
            }

            // Messaggio di join al gruppo da parte di un nuovo utente
            else if(msgType == 2){
                send(clients[i]->sockfd, mittente->nome, 30, 0);    // invio nome client che ha joinato
            }

            // Messaggio di quit dal gruppo da parte di un utente
            else if(msgType == 3){
                send(clients[i]->sockfd, mittente->nome, 30, 0);    // invio nome client che ha quittato
            }
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}