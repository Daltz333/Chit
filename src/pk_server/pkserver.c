#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include "../shared/Messages.h"
#include "../shared/DieWithError.h"

/* Represents an entry in the public key "DB" */
typedef struct {
    unsigned int user_id;
    int public_key;
} PkEntry;

/**
 * Handles a given client connection. 
 * Param incMessage: Incoming message
 * Param clntAddr: Handled socket of sender
 * Param registered_clients: Pointer to array of registered clients
 * Param sock: Binded sock
*/
void handleClient(Pk_Message *incMessage, PkEntry *registered_clients, struct sockaddr_in clntAddr, int sock) {
    printf("Received message (%i) from user (%i).\n", (int)incMessage->message_type, incMessage->user_id);

    switch (incMessage->message_type)
    {
        case REGISTER_PK:
            /* Register user & PK to memory then send ACK */
            // NOTE: didn't really feel like handling pushing/appending to an array in C
            // so this logic breaks if we have more than 100 clients registered
            for (int i = 0; i < sizeof(*registered_clients); i++)
            {
                PkEntry entry = registered_clients[i];

                if (entry.user_id == incMessage->user_id) 
                {
                    /* User already exists, log and ignore */
                    printf("Cannot register user (%i) as they already exist.\n", entry.user_id);
                } else if (entry.user_id == 0) {
                    // Entry is unused, let's fill it in
                    entry.user_id = incMessage->user_id;
                    entry.public_key = incMessage->public_key;
                    printf("Successfully registered user %i.\n", incMessage->user_id);
                }
            }

            Pk_Message *outMessage = malloc(sizeof(Pk_Message));
            memset(outMessage, 0, sizeof(*outMessage));

            outMessage->message_type = REGISTER_PK_ACK;
            outMessage->public_key = incMessage->user_id;
            outMessage->user_id = incMessage->user_id;

            /* Send ACK */
            if ((sendto(sock, outMessage, sizeof(outMessage), 0, (struct sockaddr *)&clntAddr, sizeof(clntAddr))) < 0) {
                printf("Failed to send ACK to client.\n");
            }

            free(outMessage);
            break;
        
        case FETCH_PK:
            int found = 0;
            int found_public_key = 0;

            /* loop and look for client */
            for (int i = 0; i < sizeof(*registered_clients); i++)
            {
                PkEntry entry = registered_clients[i];
                
                if (entry.user_id == incMessage->req_user_id) {
                    /* found */
                    found_public_key = entry.public_key;
                    found = 1;
                }
            }

            Pk_Message *ack = malloc(sizeof(Pk_Message));
            memset(ack, 0, sizeof(*ack));

            *ack = *incMessage; // make contents of pointer the same
            ack->message_type = FETCH_PK_ACK;
                        
            if (found == 0) 
            {
                /* NOT FOUND */
                ack->public_key = 0;
            } else {
                ack->public_key = found_public_key;
            }

            /* Send ACK */
            if ((sendto(sock, ack, sizeof(ack), 0, (struct sockaddr *)&clntAddr, sizeof(clntAddr))) < 0) {
                printf("Failed to send ACK to client.\n");
            }

            free(ack);
            break;
            
        default:
            printf("Received an unhandled message type %i. Ignoring.", (int)incMessage->message_type);
            break;
    }
}

int main(int argc, char *argv[])
{
    printf("Started Public Key Server!\n");
    
    int sock; /* Socket for receiving datagrams */
    struct sockaddr_in sockAddr; /* Represents a TCp connection */
    unsigned int serverPort = -1; /* Server Port*/
    PkEntry registered_pks[100]; /* Array of registered public keys */

    /* Clear any existing cruft */
    memset(registered_pks, 0, sizeof(registered_pks));

    if (argc != 2)    /* Test for correct number of arguments */
    {
        fprintf(stderr,"Usage: %s <Port (12000)>\n", argv[0]);
        exit(1);
    }

    serverPort = (unsigned int)strtol(argv[1], NULL, 10); /* Port */

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
        Pk_Message *incMessage = malloc(sizeof(Pk_Message));

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
        handleClient(incMessage, registered_pks, echoClntAddr, sock);

        free(incMessage);
    }
}
