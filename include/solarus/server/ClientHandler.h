#ifndef SOLARUS_CLIENTHANDLER_H
#define SOLARUS_CLIENTHANDLER_H

#include "Types.h"

class ClientHandler {
	public:
		ClientHandler(client_data* c);
		~ClientHandler();
		void Handle();
	private:
		client_data cl_d;
};

#endif
