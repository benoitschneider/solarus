#ifndef CLIENT_H
#define CLIENT_H

#include "Types.h"

class Client {
public:
	Client();
	~Client();
	bool Connect(char* srv_addr,unsigned short int port);
private:
	sockaddr_in srv_addr;
	int port;
	int sock;
};

#endif
