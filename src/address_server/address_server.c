#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <time.h>       /* for timestamps */
#include "../shared/AddressEntry.h"
#include "../shared/Messages.h"
#include "../shared/DieWithError.h"

/**
 * Handles a given client connection. 
 * Param incMessage: Incoming message
 * Param clntAddr: Handled socket of sender
 * Param registered_clients: Pointer to array of registered clients
 * Param sock: Binded sock
*/
void handleClient(Addr_Serv_Message *incMessage, AddressEntry *registered_clients, struct sockaddr_in clntAddr, int sock) {
    printf("Received message (%i) from user (%i).\n", (int)incMessage->message_type, incMessage->user_id);

    switch (incMessage->message_type)
    {
        case REGISTER_ADDR:
            /* Register user & PK to memory then send ACK */
            // NOTE: didn't really feel like handling pushing/appending to an array in C
            // so this logic breaks if we have more than 100 clients registered
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                AddressEntry *entry = &registered_clients[i];

                if (entry->user_id == incMessage->user_id) 
                {
                    /* User already exists, log and ignore */
                    printf("Cannot register user (%i) as they already exist.\n", entry->user_id);
                    break;
                } else if (entry->user_id == 0) {
                    // Entry is unused, let's fill it in
                    entry->user_id = incMessage->user_id;
                    entry->clientIp = clntAddr.sin_addr.s_addr;
                    entry->clientListenPort = incMessage->user_port;

                    printf("Successfully registered user %i.\n", incMessage->user_id);
                    break;
                }
            }

            Addr_Serv_Message *outMessage = malloc(sizeof(Addr_Serv_Message));
            memset(outMessage, 0, sizeof(*outMessage));

            outMessage->message_type = REGISTER_ADDR_ACK;
            outMessage->remote_client_ip = clntAddr.sin_addr.s_addr;
            outMessage->user_id = incMessage->user_id;
            outMessage->timestamp = (long int)time(NULL);

            /* Send ACK */
            if ((sendto(sock, outMessage, sizeof(outMessage), 0, (struct sockaddr *)&clntAddr, sizeof(clntAddr))) < 0) {
                printf("Failed to send ACK to client.\n");
            }

            free(outMessage);
            break;
        
        case FETCH_CLIENTS:
            Addr_Serv_Message *res = malloc(sizeof(Addr_Serv_Message));
            memset(res, 0, sizeof(*res));

            res->message_type = FETCH_CLIENTS_ACK;
            res->timestamp = (long int)time(NULL);

            for(int i = 0; i < MAX_CLIENTS; i++) 
            {
                AddressEntry *entry = &registered_clients[i];

                // Copy contents into response array
                res->clients[i].clientIp = entry->clientIp;
                res->clients[i].clientListenPort = entry->clientListenPort;
                res->clients[i].user_id = entry->user_id;
            }

            /* Send ACK */
            if ((sendto(sock, res, sizeof(*res), 0, (struct sockaddr *)&clntAddr, sizeof(clntAddr))) < 0) {
                printf("Failed to send ACK to client.\n");
            }

            free(res);
            break;

        case FETCH_ADDR:
            in_addr_t found_address;
            int found_port = 0;

            /* loop and look for client */
            for (int i = 0; i < sizeof(MAX_CLIENTS); i++)
            {
                AddressEntry *entry = &registered_clients[i];
                
                if (entry->user_id == incMessage->req_user_id) 
                {
                    /* found */
                    found_address = entry->clientIp;
                    found_port = entry->clientListenPort;
                    break;
                }
            }

            Addr_Serv_Message *ack = malloc(sizeof(Addr_Serv_Message));
            memset(ack, 0, sizeof(*ack));

            *ack = *incMessage; // make contents of pointer the same
            ack->message_type = FETCH_ADDR_ACK;
            ack->timestamp = (long int)time(NULL);
            ack->remote_client_ip = found_address;
            ack->remote_client_port = found_port;

            /* Send ACK */
            if ((sendto(sock, ack, sizeof(*ack), 0, (struct sockaddr *)&clntAddr, sizeof(clntAddr))) < 0) {
                printf("Failed to send ACK to client.\n");
            }

            free(ack);
            break;
            
        case FETCH_ADDR_IP:
            int found_user = 0;

            /* loop and look for client */
            for (int i = 0; i < sizeof(MAX_CLIENTS); i++)
            {
                AddressEntry *entry = &registered_clients[i];
                
                if (entry->clientIp == incMessage->remote_client_ip) 
                {
                    /* found */
                    found_user = entry->user_id;
                    break;
                }
            }

            Addr_Serv_Message *res2 = malloc(sizeof(Addr_Serv_Message));
            memset(res2, 0, sizeof(*res));

            *res2 = *incMessage; // make contents of pointer the same
            res2->message_type = FETCH_ADDR_IP_ACK;
            res2->timestamp = (long int)time(NULL);
            res2->req_user_id = found_user;

            /* Send ACK */
            if ((sendto(sock, res2, sizeof(*res2), 0, (struct sockaddr *)&clntAddr, sizeof(clntAddr))) < 0) {
                printf("Failed to send ACK to client.\n");
            }

            free(res2);
            break;

        default:
            printf("Received an unhandled message type %i. Ignoring.\n", (int)incMessage->message_type);
            break;
    }
}

int main(int argc, char *argv[])
{   
    int sock; /* Socket for receiving datagrams */
    struct sockaddr_in sockAddr; /* Represents a TCp connection */
    unsigned int serverPort = -1; /* Server Port*/
    AddressEntry registered_addr[MAX_CLIENTS]; /* Array of registered public keys */

    /* Clear any existing cruft */
    memset(registered_addr, 0, sizeof(registered_addr));

    if (argc != 2)    /* Test for correct number of arguments */
    {
        fprintf(stderr,"Usage: %s <Port (12000)>\n", argv[0]);
        exit(1);
    }

    printf("\n==================================\n");
    printf("Started Address Server!\n");
    printf("==================================\n\n");

    serverPort = (unsigned int)strtol(argv[1], NULL, 10); /* Port */

    printf("Listening on UDP %u\n\n", serverPort);

    /* Validate server port */
    if (serverPort == -1) {
        DieWithError("An invalid port has been entered. Port must be an integer be between 0 and 65353.");
    }

    /* Attempt to open up socket */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        DieWithError("Establishing socket() failed.");
    }

    memset(&sockAddr, 0, sizeof(sockAddr)); /* Zero out structure */
    sockAddr.sin_family = AF_INET; /* Internet address family */
    sockAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* any incoming interface */
    sockAddr.sin_port = htons(serverPort);

    if (bind(sock, (struct sockaddr *) &sockAddr, sizeof(sockAddr)) < 0) {
        DieWithError("Failed to bind to socket. Port may be in use.");
    }

    for (;;) {
        struct sockaddr_in echoClntAddr;
        Addr_Serv_Message *incMessage = malloc(sizeof(Addr_Serv_Message));

        int recvMsgSize;

        printf("Waiting for message from client.\n");

        int clntLen = sizeof(&echoClntAddr);
        /* Block until receive message from a client */
        if ((recvMsgSize = recvfrom(sock, incMessage, sizeof(*incMessage), 0,
                                    (struct sockaddr *) &echoClntAddr, (socklen_t *)&clntLen)) < 0)
        {
            DieWithError("recvfrom() failed.");
        }

        /* Delegate to function to keep main() clean */
        handleClient(incMessage, registered_addr, echoClntAddr, sock);

        free(incMessage);
    }
}
