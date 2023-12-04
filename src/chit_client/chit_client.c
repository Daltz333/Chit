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
#include "ThreadArgs.h"

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
   if ((recvMsgSize = recvfrom(sock, message, sizeof(Pk_Message), 0,
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

void exitFromAll(int userId, struct sockaddr_in *addrSock, struct sockaddr_in *pkSock, int sock)
{
   Pk_Message disconnectPk;
   disconnectPk.message_type = PK_EXIT;
   disconnectPk.timestamp = (long int)time(NULL);
   disconnectPk.user_id = userId;

   /* We do not care about response */
   getPkMessage(&disconnectPk, pkSock, sock);

   Addr_Serv_Message disconnectAddr;
   disconnectAddr.message_type = PK_EXIT;
   disconnectAddr.timestamp = (long int)time(NULL);
   disconnectAddr.user_id = userId;
   
   /* We do not care about the response */
   getAddrMessage(&disconnectAddr, addrSock, sock);
}

/**
 * Fetches a given user from the connected IP addr
*/
int fetchUserFromIp(struct sockaddr_in *addrSock, struct in_addr ipAddr, int sock)
{
   Addr_Serv_Message *message = malloc(sizeof(Addr_Serv_Message));
   memset(message, 0, sizeof(Addr_Serv_Message));
   
   message->message_type = FETCH_ADDR_IP;
   message->remote_client_ip = ipAddr.s_addr;
   message->timestamp = (long int)time(NULL);
   
   getAddrMessage(message, addrSock, sock);

   if (message->message_type == (int)FETCH_ADDR_IP_ACK)
   {
      return message->req_user_id;
   } else
   {
      return 0;
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
      printf("accept/deny - Accepts or denies an incoming connection request\n");
      printf("quit - Exists the current connection\n");
      printf("====================================\n\n");
}

int startChat(int user_id, struct sockaddr_in *addrServSock, int sock, ThreadArgs *targs, int *connected_user_id)
{
   int connectUserId;
   askQuestionInt("Enter the user ID to connect to: ", &connectUserId);

   if (connectUserId == 0)
   {
      printf("Invalid user ID.\n");
      return -1;
   }

   Addr_Serv_Message *addrMessage = malloc(sizeof(Addr_Serv_Message));
   memset(addrMessage, 0, sizeof(sizeof(Addr_Serv_Message)));

   addrMessage->message_type = FETCH_ADDR;
   addrMessage->req_user_id = connectUserId;
   addrMessage->user_id = user_id;

   getAddrMessage(addrMessage, addrServSock, sock);

   int clientSock = socket(AF_INET, SOCK_STREAM, 0);
   if (clientSock == -1) {
      printf("Error creating socket");
      return -1; 
   }

   // Set up server address structure
   struct sockaddr_in serverAddr;
   memset(&serverAddr, 0, sizeof(serverAddr));

   if (addrMessage->remote_client_ip == 0)
   {
      printf("Failed to retrieve the remote IP from addr server.\n");
      return -1;
   }
   
   serverAddr.sin_family = AF_INET;
   serverAddr.sin_port = htons(addrMessage->remote_client_port);
   serverAddr.sin_addr.s_addr = addrMessage->remote_client_ip;

   if (connect(clientSock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
   {
      printf("Failed to establish connection with server\n");
      return -1;
   }

   // Copy user id into ref
   *connected_user_id = connectUserId;
   targs->clientSock = clientSock;
   free(addrMessage);
   return 0;
}

/**
 * Fetches the public key for a given user
 * Returns the public key, or -1 if not found
*/
int getPubKey(int remoteUserId, struct sockaddr_in *pkSock, int sock)
{
   Pk_Message *pubKey = malloc(sizeof(Pk_Message));
   memset(pubKey, 0, sizeof(Pk_Message));

   pubKey->message_type = FETCH_PK;
   pubKey->req_user_id = remoteUserId;
   pubKey->timestamp = (long int)time(NULL);

   getPkMessage(pubKey, pkSock, sock);

   // Copy to buffer, then free
   int message = pubKey->message_type;
   int key = pubKey->public_key;
   free(pubKey);

   if (message == (int)FETCH_PK_ACK)
   {
      return key;
   } else 
   {
      printf("Unknown response from PK server, %i\n", message);
      return -1;
   }
}

void sendMsg(ThreadArgs *targs, struct sockaddr_in *pkServSock, int sock, int user_id)
{
   char msg[MAX_MESSAGE_SIZE];
   askQuestion("Type your message: ", msg, "err", MAX_MESSAGE_SIZE);

   if (strcmp(msg, "err") == 0) 
   {
      return;
   }

   int user_pub_key = getPubKey(user_id, pkServSock, sock);

   unsigned long payload[MAX_MESSAGE_SIZE] = {0};
   encryptMessage(payload, msg, sizeof(msg), user_pub_key);

   int sentBytes = send(targs->clientSock, payload, MAX_MESSAGE_SIZE, 0);
   if (sentBytes == -1)
   {
      printf("Failed to send message to target.");
   }

   printf("Sent %i bytes.\n", sentBytes);
}

/**
 * Loop continuously blocking for user input
*/
void processStandardIn(int user_id, struct sockaddr_in *addrServSock, struct sockaddr_in *pkServSock, int sock, ThreadArgs *targs)
{
   char command[20];
   int connected_user_id = 0;
   pthread_t thread_id;

   for (;;)
   {
      memset(command, 0, sizeof(command));
      askQuestion("Enter in a command, or type help for a list of commands: \n", command, "help", 20);

      if (strcmp(command, "help") == 0) 
      {
         printHelp();
      } else if (strcmp(command, "who") == 0)
      {
         printAvailUsers(user_id, addrServSock, sock);
      } else if (strcmp(command, "connect") == 0)
      {
         int res = startChat(user_id, addrServSock, sock, targs, &connected_user_id);
         if (res == 0)
         {
            pthread_create(&thread_id, NULL, listenForMessages, targs);
         }
      } else if (strcmp(command, "accept") == 0)
      {
         if (targs->ConnectStatus == (int)WAITING)
         {
            targs->ConnectStatus = ACCEPTED;
         } else 
         {
            printf("No incoming connection\n");
         }
      } else if (strcmp(command, "sendmsg") == 0)
      {
         /* User_id should only ever be zero if we are client connected */
         if (connected_user_id == 0)
         {
            connected_user_id = fetchUserFromIp(addrServSock, targs->connectedAddr, sock);
         }

         if (targs->clientSock != 0)
         {
            sendMsg(targs, pkServSock, sock, connected_user_id);
         } else {
            printf("Not currently connected!\n");
         }
      } else if (strcmp(command, "deny") == 0)
      {
         if (targs->ConnectStatus == (int)WAITING)
         {
            targs->ConnectStatus = DENIED;
         } else 
         {
            printf("No incoming connection\n");
         }
      } else if (strcmp(command, "quit") == 0)
      {
         exitFromAll(user_id, addrServSock, pkServSock, sock);

         pthread_cancel(thread_id);
         pthread_join(thread_id, NULL);
         printf("Stopped listening for connections\n");
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

   /* Initialize structure to hold our cross thread args */
   ThreadArgs *targs = malloc(sizeof(ThreadArgs));
   memset(targs, 0, sizeof(ThreadArgs));
   targs->listeningPort = listeningPort;
   targs->prvKey = prvKey;
   
   /* Listen for incoming connection requests in another thread */
   pthread_t thread_id;
   pthread_create(&thread_id, NULL, startServThread, targs);

   /* Wait for server thread to start */
   sleep(1);

   /* Block main thread for regular user input */
   processStandardIn(userId, &addrSockAddr, &pkSockAddr, sock, targs);

   /* Cleanup, we're done */
   free(targs);
}