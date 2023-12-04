#pragma once

typedef struct {
    int clientSock;
    int listeningPort;
    unsigned long *messageToSend;
    enum { ACCEPTED, DENIED, WAITING, IDLE } ConnectStatus;
} ThreadArgs;