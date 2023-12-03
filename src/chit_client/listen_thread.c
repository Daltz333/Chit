#include <unistd.h>     /* Sleep */
#include <stdio.h>      /* Printf */
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include "ThreadArgs.h"
#include "../shared/StringUtil.h"
#include "../shared/Messages.h"

/**
 * Start listening for any incoming connections
 * Params: Pointers to state that we poll for requests from the main thread
 * requestConnect -1 for when we are exiting cleanly
*/
void *startListening(void *vargp)
{
    ThreadArgs *args = vargp;
    int socket_desc = 0;
    int client_sock = 0;
    socklen_t client_size = 0;
    struct sockaddr_in client_addr;

    Chit_Message *message = malloc(sizeof(Chit_Message));
    memset(message, 0, sizeof(Chit_Message));
    
    for(;;)
    {
        // Accept an incoming connection:
        client_size = sizeof(client_addr);
        client_sock = accept(socket_desc, (struct sockaddr*)&client_addr, &client_size);
        
        // Clean buffers:
        memset(memset, 0, sizeof(Chit_Message));

        if (client_sock < 0){
            printf("Can't accept\n");
            continue;;
        }

        printf("Connection request at IP: %s and port: %i\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        char response[20];
        askQuestion("Type accept to confirm, or deny to reject.", response, "deny", 20);

        if (strcmp(response, "accept") == 0)
        {
            *args->isConnected = 1;

            // Connection request allowed, accept, wait for messages
            for(;;)
            {
                if (*args->isConnected != 1)
                {
                    printf("Exiting communication\n");
                    break;
                }

                if (recv(client_sock, message, sizeof(message), 0) < 0){
                    printf("Couldn't receive\n");
                    continue;
                 }

                printf("Message from %i: %lu", message->user_id, *message->payload);
            }
        }

        close(client_sock);
        close(socket_desc);
    }
}