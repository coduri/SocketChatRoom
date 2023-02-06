// Created by Christian Coduri on 04/02/23.
#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_RESET   "\x1b[0m"


void printRed(char *str) {
    printf(COLOR_RED "%s" COLOR_RESET, str);
}

void printGreen(char *str) {
    printf(COLOR_GREEN "%s" COLOR_RESET, str);
}

void printCyan(char *str) {
    printf(COLOR_CYAN "%s" COLOR_RESET, str);
}

void printYellow(char *str) {
    printf(COLOR_YELLOW "%s" COLOR_RESET, str);
}

void clearInput() {
    printf("\33[2K\r");
}