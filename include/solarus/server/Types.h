#ifndef TYPES_H
#define TYPES_H

#define SERVER_PORT 15000
#define MAXPENDING 4

#include <unistd.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <string.h> 
#include <iostream>

typedef struct server_data {
        struct sockaddr_in server_addr;
        int port;
        int socket;
        int maxpending;
        bool ready;
} server_data;


typedef struct client_data {
        struct sockaddr_in client_addr;
        int socket;
} client_data;

using namespace std;
#endif
