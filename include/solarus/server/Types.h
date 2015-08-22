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
