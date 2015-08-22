#ifndef SOLARUS_SERVER_H
#define SOLARUS_SERVER_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <string.h>

using namespace std;

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



class Server {
	public:
		Server(int port,int maxpending);
		~Server();
		bool Connect();
		void Run();
	private:
		server_data* srv_d;
};

#endif
