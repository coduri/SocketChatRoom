// Created by Christian Coduri on 04/02/23.

#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_RESET   "\x1b[0m"
#define COLOR_CYAN    "\x1b[36m"


void printRed(char* str){
    printf(COLOR_RED "%s" COLOR_RESET, str);
}

void printGreen(char* str){
    printf(COLOR_GREEN "%s" COLOR_RESET, str);
}


void printMessage(char* name, char* msg){
    printf(COLOR_CYAN);
    printf("%s: ", name);
    printf(COLOR_RESET);

    printf("%s", msg);
}

void printUserJoin(char* name){
    char formatString[100];
    snprintf(formatString, sizeof(formatString), "=== %s si è unito alla chat ===\n", name);

    printGreen(formatString);
}

void printUserLeft(char* name){
    char formatString[100];
    snprintf(formatString, sizeof(formatString), "=== %s ha abbandonato la chat ===\n", name);

    printRed(formatString);
}