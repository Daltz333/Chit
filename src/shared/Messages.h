#include <stdint.h>
#include "AddressEntry.h"
#include "Globals.h"

typedef uint32_t in_addr_t;

/** Represents a message from a Chit client to another Chit client */
typedef struct {
    enum { MESSAGE, CHIT_EXIT } message_type; /* Message type */
    unsigned long int timestamp; /* Timestamp of the sent message */
    unsigned int user_id; /* Sender ID of the message */
    unsigned long *payload; /* Encrypted message contents */
} Chit_Message;

/** Represents a message to or from the Public Key Server */
typedef struct {
    enum {FETCH_PK, FETCH_PK_ACK, REGISTER_PK, REGISTER_PK_ACK, PK_EXIT} message_type; /* Message type */
    unsigned long int timestamp; /* Timestamp of the sent message */
    unsigned int user_id; /* Id of the user who sent the message, if applicable */

    /* If fetch_pk, this is requested user id for pk request */
    unsigned int req_user_id;

     /* if fetch_pk, this is 0 */
     /* if fetch_pk_ack, this is PK of request */
     /* if register_pk, this is PK of sender */
     /* if register_pk_ack, this is PK of sender or 0 if not found */
    unsigned int public_key;
} Pk_Message;

/** Represents a message to or from the Address Server */
typedef struct {
    enum {REGISTER_ADDR, REGISTER_ADDR_ACK, FETCH_ADDR, FETCH_ADDR_ACK, FETCH_CLIENTS, FETCH_CLIENTS_ACK, ADDR_EXIT} message_type; /* Message type */
    unsigned long int timestamp; /* Timestamp of the sent message */
    unsigned int user_id; /* Id of the user who sent the message, if applicable */
    unsigned int req_user_id; /* Id of the user we want to fetch address for */
    AddressEntry clients[MAX_CLIENTS]; /* List of clients currently connected */
    in_addr_t remote_client_ip; /* IP address of the user we fetched */
    unsigned int remote_client_port; /* Port of the user we fetched */
    unsigned int user_port; /* Port address of the chit client in the case of REGISTER_ACK */
} Addr_Serv_Message;
