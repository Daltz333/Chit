#pragma once

#include <arpa/inet.h>

/**
 * Kinda a catch all for global application state
*/
typedef struct {
    int clientSock;
    int listeningPort;
    int prvKey;
    unsigned long *messageToSend;
    struct in_addr connectedAddr;
    enum { ACCEPTED, DENIED, WAITING, IDLE, EXITED } ConnectStatus;
} ThreadArgs;