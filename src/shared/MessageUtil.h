#include <stddef.h>

int encryptMessage(unsigned long *buffer, char* msg, size_t bufferSize);
int decryptMessage(char* decryptedMsg, int* encryptedMsg);
int getTimestamp();
long long int encrypt(int msg, int prvKey);
long long int decrypt(int msg, int pubKey);
