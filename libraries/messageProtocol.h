// Created by Christian Coduri on 05/02/23.
#define MAX_MESSAGE_LENGTH 100
#define MAX_SENDER_NAME_LENGTH 32
#define SIZE_OF_MESSAGE_PROTOCOL sizeof(messageProtocol)


typedef enum {
    MESSAGE_TYPE_JOIN,
    MESSAGE_TYPE_LEAVE,
    MESSAGE_TYPE_SEND,
} messageType;

typedef struct {
    messageType typeMessage;
    char sender[MAX_SENDER_NAME_LENGTH];
    char message[MAX_MESSAGE_LENGTH];
} messageProtocol;


