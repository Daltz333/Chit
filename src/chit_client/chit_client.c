#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <stdio.h>      /* for console input */
#include <time.h>       /* for system time */
#include <stdlib.h>     /* for string manip */
#include "../shared/DieWithError.h"
#include "../shared/Messages.h"
#include "../shared/MessageUtil.h"
#include "../shared/StringUtil.h"

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

/** Registers the users public key to the public key server */
void registerPublicKey(char *pkIp, int pkPort, int pubKey)
{
   
}

void registerAddress(char *addrServIp, int addrServPort, int listeningPort)
{

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

   char pkIp[20];
   int pkPort = 0;
   char addrServIp[20];
   int addrServPort = 0;
   int pubKey = 0;
   int prvKey = 0;
   requestConfigurationDetails(pkIp, &pkPort, addrServIp, &addrServPort, &pubKey, &prvKey);

   printf("\n Register public key with Public Key Server.\n");
   registerPublicKey(pkIp, pkPort, pubKey);

   printf("\n Registering client with Address Server\n");
   registerAddress(addrServIp, addrServPort, listeningPort);
}