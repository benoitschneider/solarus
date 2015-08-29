#include "solarus/server/ClientHandler.h"
#define BUFFSIZE 256

ClientHandler::ClientHandler(client_data* c){
	this->cl_d.client_addr = c->client_addr;
	this->cl_d.socket = c->socket;

	this->Handle();
}

void ClientHandler::Handle(){
	cout << "Client " << inet_ntoa(this->cl_d.client_addr.sin_addr) << " connected" << endl;

	string cmd;
	char buffer[BUFFSIZE];
	int n;
	while((n = read(this->cl_d.socket, buffer, BUFFSIZE)) > 0) {
		cmd = string (buffer);
		cout << cmd;
		bzero(buffer,BUFFSIZE);
	}
}

ClientHandler::~ClientHandler(){
	if (this->cl_d.socket > 0) {
		close(this->cl_d.socket);
	}
}
