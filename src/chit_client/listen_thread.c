#include <unistd.h>     /* Sleep */
#include <stdio.h>      /* Printf */
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include "ThreadArgs.h"
#include "../shared/StringUtil.h"
#include "../shared/Messages.h"

/* Begins listening for messages until args -> ConnectStatus = EXIT*/
void listenForMessages(void *vargp)
{
    ThreadArgs *args = vargp;
    char buffer[MAX_MESSAGE_SIZE];

    // Connection request allowed, accept, wait for messages
    if (args->ConnectStatus == (int)ACCEPTED)
    {
        args->ConnectStatus = IDLE; // reset state
        for(;;)
        {
            if (args->ConnectStatus == (int)EXITED)
            {
                args->ConnectStatus = IDLE;
                break;
            }

            memset(buffer, '\0', sizeof(buffer));
            read(args->clientSock, buffer, MAX_MESSAGE_SIZE);

            char clnData[MAX_MESSAGE_SIZE];
            memset(clnData, '\0', MAX_MESSAGE_SIZE);

            int x = 0;
            for (int i = 0; i < MAX_MESSAGE_SIZE; i++)
            {
                /* for some reasons, there are gaps in the message, let's fix that */
                if ((int)buffer[i] > 0)
                {
                    clnData[x] = buffer[i];
                    x += 1;
                }
            }

            if (strlen(clnData) != 0)
            { 
                printf("Received msg: %s\n", clnData);
            }
        }
    }
}

/**
 * Start listening for any incoming connections
 * Params: Pointers to state that we poll for requests from the main thread
 * requestConnect -1 for when we are exiting cleanly
*/
void *startServThread(void *vargp)
{
    ThreadArgs *args = vargp;
    int socket_desc = 0;
    int server_sock = 0;
    int client_sock = 0;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;

    Chit_Message *message = malloc(sizeof(Chit_Message));
    memset(message, 0, sizeof(Chit_Message));
    
    // Create socket
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(args->listeningPort);

    // Bind the socket
    if (bind(server_sock, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        printf("Bind failed\n");
    }

    for(;;)
    {
        // Listen for incoming connections
        if (listen(server_sock, 3) == -1) {
            printf("Listen failed\n");
            exit(EXIT_FAILURE);
        }

        printf("TCP listening on port %d\n", args->listeningPort);

        // Accept a connection
        socklen_t client_address_len = sizeof(client_address);
        if ((client_sock = accept(server_sock, (struct sockaddr*)&client_address, &client_address_len)) == -1) {
            printf("Accept failed\n");
            exit(EXIT_FAILURE);
        }

        if (args->clientSock != 0)
        {
            printf("An incoming connection request has been rejected. Already in an existing chat session.\n");
            continue;

            close(client_sock);
            close(socket_desc);
        }

        args->ConnectStatus = WAITING;
        printf("\nConnection request %s:%d. Type accept/deny to confirm.\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

        args->clientSock = client_sock;

        if (server_sock < 0)
        {
            sleep(1); // no incoming connections
            continue;
        }

        for(;;)
        {
            if (args->ConnectStatus == (int)ACCEPTED || args->ConnectStatus == (int)DENIED)
            {
                break; // confirmation 
            }

            sleep(1);
        }

        if (args->ConnectStatus == (int)ACCEPTED)
        {
            listenForMessages((void *)args);
        }

        close(server_sock);
        close(socket_desc);
    }
}