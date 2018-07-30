#include "cpp_client.h"

#include <iostream>



int main(int nchar, char *schar[]) {

	std::string address = "127.0.0.1";
	int port = 5001;

	//TODO parse the shell args so that the address can point elsewhere

	Client client(address, port);


	while(1) {
		//TODO loop parsing/sending command via the client
		//TODO define an esc char to bail out
	}

	return 0;
}
