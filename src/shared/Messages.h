/** Represents a message from a Chit client to another Chit client */
typedef struct {
    enum { MESSAGE } message_type; /* Message type */
    unsigned long int timestamp; /* Timestamp of the sent message */
    unsigned int user_id; /* Sender ID of the message */
    unsigned long *payload; /* Encrypted message contents */
} Chit_Message;

/** Represents a message to or from the Public Key Server */
typedef struct {
    enum {FETCH_PK, FETCH_PK_ACK, REGISTER_PK, REGISTER_PK_ACK} message_type; /* Message type */
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
    enum {REGISTER_ADDR, REGISTER_ADDR_ACK, FETCH_ADDR, FETCH_ADDR_ACK} message_type; /* Message type */
    unsigned long int timestamp; /* Timestamp of the sent message */
    unsigned int user_id; /* Id of the user who sent the message, if applicable */
    unsigned int user_port; /* Port address of the chit client in the case of REGISTER_ACK */
    char* user_addr; /* IP address of the chit client in the case of FETCH_ADDR */ 
} Addr_Serv_Message;
