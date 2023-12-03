#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <stdio.h>      /* for console input */
#include <time.h>       /* for system time */
#include <stdlib.h>     /* for string manip */
#include <time.h>       /* User ID generation and timestamp */
#include <pthread.h>    /* Threading */
#include "../shared/DieWithError.h"
#include "../shared/Messages.h"
#include "../shared/MessageUtil.h"
#include "../shared/StringUtil.h"
#include "../shared/Globals.h"
#include "listen_thread.h"

void getPkMessage(Pk_Message *message, struct sockaddr_in *pkServSock, int sock)
{
   /* Send MSG */
   if ((sendto(sock, message, sizeof(Pk_Message), 0, (struct sockaddr *)pkServSock, sizeof(*pkServSock))) < 0) {
         printf("Failed to send ACK to client.\n");
   }

   ssize_t recvMsgSize;
   struct sockaddr_in echoClntAddr;
   int clntLen = sizeof(&echoClntAddr);

   /* Block until receive message from server */
   if ((recvMsgSize = recvfrom(sock, message, sizeof(*message), 0,
                              (struct sockaddr *) &echoClntAddr, (socklen_t *)&clntLen)) < 0)
   {
      DieWithError("recvfrom() failed.");
   }
}

void getAddrMessage(Addr_Serv_Message *message, struct sockaddr_in *addrServSock, int sock)
{
   /* Send MSG */
   if ((sendto(sock, message, sizeof(Addr_Serv_Message), 0, (struct sockaddr *)addrServSock, sizeof(*addrServSock))) < 0) {
         printf("Failed to send ACK to client.\n");
   }

   ssize_t recvMsgSize;
   struct sockaddr_in echoClntAddr;
   int clntLen = sizeof(&echoClntAddr);

   /* Block until receive message from server */
   if ((recvMsgSize = recvfrom(sock, message, sizeof(*message), 0,
                              (struct sockaddr *) &echoClntAddr, (socklen_t *)&clntLen)) < 0)
   {
      DieWithError("recvfrom() failed.");
   }
}

void requestConfigurationDetails(char *pkIp, int *pkPort, char *addrServIp, int *addrServPort, int *pubKey, int *prvKey)
{
   memset(pkIp, 0, sizeof(*pkIp)); // clear out any cruft

   int isValidIp = 0;
   while (isValidIp == 0)
   {
      askQuestion("Public Key Server IP (enter for default): ", pkIp, "127.0.0.1", 20);
      if (isValidIpAddress(pkIp) == 0) 
      {
         isValidIp = 1;
      } else {
         printf("Provided IP is invalid.\n");
      }
   }

   while (*pkPort == 0)
   {
      askQuestionInt("Public Key Server Port: ", pkPort);
   }

   memset(addrServIp, 0, sizeof(*pkIp)); // clear out any cruft

   isValidIp = 0;
   while (isValidIp == 0)
   {
      askQuestion("Address Server IP (enter for default): ", addrServIp, "127.0.0.1", 20);
      if (isValidIpAddress(addrServIp) == 0) 
      {
         isValidIp = 1;
      } else {
         printf("Provided IP is invalid.\n");
      }
   }

   while (*addrServPort == 0) 
   {  
      askQuestionInt("Address Server Port (enter for default): ", addrServPort);
   }

   while (*pubKey == 0)
   {
      askQuestionInt("Enter in your public key: ", pubKey);
   }

   while (*prvKey == 0) 
   {
      askQuestionInt("Enter in your private key: ", prvKey);
   }
}

/** 
 * Registers the users public key to the public key server. 
 */
void registerPublicKey(int pubKey, int userId, struct sockaddr_in *targetSock, int sock)
{
   Pk_Message *message = malloc(sizeof(Pk_Message));
   memset(message, 0, sizeof(Pk_Message));
   
   message->message_type = REGISTER_PK;
   message->public_key = pubKey;
   message->user_id = userId;
   message->timestamp = (long int)time(NULL);
   
   getPkMessage(message, targetSock, sock);

   if (message->message_type == (int)REGISTER_PK_ACK)
   {
      printf("Successfully registered with PK server.\n");
   } else {
      DieWithError("Failed to register with PK server.\n");
   }

   free(message);
}

/**
 * Registers the users address and port to the address lookup server.
*/
void registerAddress(int listeningPort, int userId, struct sockaddr_in *addrSock, int sock)
{
   Addr_Serv_Message *message = malloc(sizeof(Addr_Serv_Message));
   memset(message, 0, sizeof(Addr_Serv_Message));
   
   message->message_type = REGISTER_ADDR;
   message->user_port = listeningPort;
   message->user_id = userId;
   message->timestamp = (long int)time(NULL);
   
   getAddrMessage(message, addrSock, sock);

   if (message->message_type == (int)REGISTER_ADDR_ACK)
   {
      printf("Successfully registered with address server.\n");
   } else
   {
      DieWithError("Failed to register with address server.\n");
   }

   free(message);
}

/**
 * Prints the available users to stdout
*/
void printAvailUsers(int userId, struct sockaddr_in *addrSock, int sock)
{
   Addr_Serv_Message *message = malloc(sizeof(Addr_Serv_Message));
   memset(message, 0, sizeof(Addr_Serv_Message));

   message->message_type = FETCH_CLIENTS;
   message->user_id = userId;
   message->timestamp = (long int)time(NULL);

   getAddrMessage(message, addrSock, sock);

   if (message->message_type == (int)FETCH_CLIENTS_ACK)
   {
      printf("\nAvailable users: \n\n");
      for (int i = 0; i < MAX_CLIENTS; i++)
      {
         AddressEntry *entry = &message->clients[i];

         /* Don't print ourselves */
         if (entry->user_id != userId && entry->user_id != 0)
         {
            printf("User: %i\n", entry->user_id);
         }
      }
      printf("\n");
   } else
   {
      printf("Server responded with an invalid response code. %i\n", message->message_type);
   }

   free(message);
}

/**
 * Prints the help text
*/
void printHelp()
{
      printf("\nThe following are valid commands: \n");
      printf("====================================\n\n");
      printf("help - Gets a list of commands\n");
      printf("who - Gets a list of connected users\n");
      printf("connect - Sends a connection request to a given user ID\n");
      printf("sendmsg - Sends a message to the given user\n");
      printf("quit - Exists the current connection\n");
      printf("====================================\n\n");
}

/**
 * Loop continuously blocking for user input
*/
void processStandardIn(int user_id, struct sockaddr_in *addrServSock, struct sockaddr_in *pkServSock, int sock)
{
   char command[20];
   for (;;)
   {
      memset(command, 0, sizeof(command));
      askQuestion("Enter in a command, or type help for a list of commands: ", command, "help", 20);

      if (strcmp(command, "help") == 0) 
      {
         printHelp();
      } else if (strcmp(command, "who") == 0)
      {
         printAvailUsers(user_id, addrServSock, sock);
      } else if (strcmp(command, "connect") == 0)
      {
         printf("unimplemented\n");
      } else if (strcmp(command, "sendmsg") == 0)
      {
         printf("unimplemented\n");
      } else if (strcmp(command, "quit") == 0)
      {
         printf("unimplemented\n");
      } else 
      {
         // unknown, print help
         printHelp();
      }
   }
}

int main(int argc, char *argv[])
{
   if (argc != 2) 
   {
        fprintf(stderr,"Usage: %s <Listening Port (12000)>\n", argv[0]);
        return -1;
   }

   int listeningPort = atoi(argv[1]);
   
   printf("\n==============================\n");
   printf("Welcome to Chit!\n");
   printf("A TCP based encrypted messaging client written in C.\n");
   printf("==============================\n\n");

   printf("Listening on TCP %i\n\n", listeningPort);

   char pkIp[20];
   int pkPort = 0;
   char addrServIp[20];
   int addrServPort = 0;
   int pubKey = 0;
   int prvKey = 0;
   requestConfigurationDetails(pkIp, &pkPort, addrServIp, &addrServPort, &pubKey, &prvKey);

   /* Initialize rand */
   srand(time(NULL));
   int userId = rand() % 5001; // generate ID from 0 -> 5000
   printf("Generated user ID {%i}.\n", userId);

   /* Configure listening sock */
   int sock = 0;
   struct sockaddr_in sockAddr;

   memset(&sockAddr, 0, sizeof(sockAddr)); /* Zero out structure */
   sockAddr.sin_family = AF_INET; /* Internet address family */
   sockAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* any incoming interface */
   sockAddr.sin_port = htons(listeningPort);

   /* Build PK server sockAddr */
   struct sockaddr_in pkSockAddr;
   memset(&pkSockAddr, 0, sizeof(pkSockAddr));
   pkSockAddr.sin_family = AF_INET;
   pkSockAddr.sin_addr.s_addr = inet_addr(pkIp);
   pkSockAddr.sin_port = htons(pkPort);

   /* Build address server sockAddr */
   struct sockaddr_in addrSockAddr;
   memset(&addrSockAddr, 0, sizeof(addrSockAddr));
   addrSockAddr.sin_family = AF_INET;
   addrSockAddr.sin_addr.s_addr = inet_addr(addrServIp);
   addrSockAddr.sin_port = htons(addrServPort);

   /* Attempt to open up socket */
   if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        DieWithError("Establishing socket() failed.\n");
   }

   if (bind(sock, (struct sockaddr *) &sockAddr, sizeof(sockAddr)) < 0) {
      DieWithError("Failed to bind to socket. Port may be in use.\n");
   }

   /* Register information with PK and Address servers*/
   printf("Registering public key with Public Key Server.\n");
   registerPublicKey(pubKey, userId, &pkSockAddr, sock);

   printf("Registering client with Address Server\n");
   registerAddress(listeningPort, userId, &addrSockAddr, sock);

   /* Listen for incoming connection requests in another thread */
   pthread_t thread_id;
   pthread_create(&thread_id, NULL, startListening, NULL);

   /* Block main thread for regular user input */
   processStandardIn(userId, &addrSockAddr, &pkSockAddr, sock);
}