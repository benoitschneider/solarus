#ifndef SOLARUS_SERVER_H
#define SOLARUS_SERVER_H

#include "Types.h"
#include "ClientHandler.h"

class Server {
	public:
		Server(unsigned short int port,unsigned short int maxpending);
		~Server();
		bool Connect();
		void Run();
	private:
		server_data* srv_d;
};

#endif
