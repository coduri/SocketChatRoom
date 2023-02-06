// Created by Christian Coduri on 05/02/23.
#include "stdoutUtil.h"

void printMessage(char* name, char* msg){
    printCyan(name);
    printf(": %s", msg);
    printf("\n");
}

void printSendingInterface(char* name){
    printYellow(name);
    printf(": ");
}

void printUserJoin(char* name){
    char formatString[100];
    sprintf(formatString, "=== %s has joined the chat ===", name);

    printGreen(formatString);
    printf("\n");
}

void printUserLeft(char* name){
    char formatString[100];
    sprintf(formatString, "=== %s has left the chat ===", name);

    printRed(formatString);
    printf("\n");
}


void printErrorStatus(char* message){
    printRed("ERROR: ");
    printf("%s", message);
    printf("\n");
}

void printOKStatus(char* message){
    printGreen("OK: ");
    printf("%s", message);
    printf("\n");
}