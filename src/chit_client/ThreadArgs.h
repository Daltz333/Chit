#pragma once

typedef struct {
    int *isConnected;
    int *socket_desc;
    unsigned long *messageToSend;
    enum { WAITING, ACCEPTED, DENIED, IDLE } ConnectStatus;
} ThreadArgs;