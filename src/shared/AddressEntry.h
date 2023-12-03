#include <arpa/inet.h>

#pragma once

/* Represents an entry in the address "DB" */
typedef struct {
    /**
     * Dotted quad IP address.
     * We opt to use this over storing the sock as I figured that'd be more expensive
    */
    in_addr_t clientIp; // dotted quad IP address

    // Port that the client is listening on
    unsigned int clientListenPort;

    // ID of the user
    unsigned int user_id;
} AddressEntry;
