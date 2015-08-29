#include "solarus/server/Server.h"
#include "ClientHandler.cpp"

Server::Server(unsigned short int port,unsigned short int maxpending) {
	this->srv_d->port = port;
	this->srv_d->maxpending = maxpending;
	this->srv_d->ready = this->Connect();
	this->Run();
}

bool Server::Connect() {
    if ((this->srv_d->port > 0) && (this->srv_d->maxpending > 0)) {
        if ((this->srv_d->socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
            return false;
        }

        memset(&this->srv_d->server_addr, 0, sizeof(this->srv_d->server_addr));
        this->srv_d->server_addr.sin_family = AF_INET;
        this->srv_d->server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        this->srv_d->server_addr.sin_port = htons(this->srv_d->port);

        if (bind(this->srv_d->socket, (const sockaddr *) &this->srv_d->server_addr, sizeof(this->srv_d->server_addr)) < 0) {
            	cout << "Port not free" << endl;
		return false;
        }

        if (listen(this->srv_d->socket, this->srv_d->maxpending) < 0) {
            return false;
        }
    } else {
        return false;
    }

    return true;
}

void Server::Run() {
	if (this->srv_d->ready){
		client_data cl_d;
		cout << "Server running in process " << getpid() << endl;
		while (this->srv_d->ready) {
			unsigned int clientlen = sizeof(cl_d.client_addr);
			// Wait for client connection
			if ((cl_d.socket = accept(this->srv_d->socket, (struct sockaddr *) &cl_d.client_addr, &clientlen)) < 0) {
				break;		
			}else{
				int pid_c;
				if ((pid_c=fork())==0) {
					ClientHandler ch = ClientHandler(&cl_d);
				}
			}
		}
	}else{
		cout << "Server not connected" << endl;
	}
}

// TODO
Server::~Server(){
	close(this->srv_d->socket);
}
