#include "solarus/server/Client.h"

Client::Client(){
}

Client::~Client(){
}

bool Client::Connect(char* srv_addr, unsigned short int port){
	if((this->sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		cout << "Client socket not created" << endl;
		return false;
	} 

	memset(&this->srv_addr, '0', sizeof(this->srv_addr)); 
	this->srv_addr.sin_family = AF_INET;
	this->srv_addr.sin_port = htons(port); 

	if(inet_pton(AF_INET, srv_addr, &this->srv_addr.sin_addr)<=0){
		cout << "inet_pton failed" << endl;
		return false;
	} 

	if( connect(this->sock, (struct sockaddr *)&this->srv_addr, sizeof(this->srv_addr)) < 0){
		cout << "Client connection failed" << endl;
		return false;
	}

	return true; 
}
