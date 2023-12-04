#include <stddef.h>

int encryptMessage(unsigned long *buffer, char* msg, size_t bufferSize, int key);
int decryptMessage(char* decryptedMsg, unsigned long *encryptedMsg, int key);
int getTimestamp();
long long int encrypt(int msg, int prvKey);
long long int decrypt(int msg, int pubKey);
