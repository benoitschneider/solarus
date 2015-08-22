#ifndef SOLARUS_SERVER_H
#define SOLARUS_SERVER_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <string.h>
#include "Types.h"

using namespace std;

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
