#include "solarus/server/ClientHandler.h"

ClientHandler::ClientHandler(client_data* c){
	this->cl_d.client_addr = c->client_addr;
	this->cl_d.socket = c->socket;
}

void ClientHandler::Handle(){
	cout << "Client " << inet_ntoa(this->cl_d.client_addr.sin_addr) << " connected" << endl;
}

ClientHandler::~ClientHandler(){
	if (this->cl_d.socket > 0) {
		close(this->cl_d.socket);
	}
}
