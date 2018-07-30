#include "cpp_client.h"

#include <iostream>
#include <sstream>


int main(int argc, char *argv[]) {

	std::string address = "127.0.0.1";
	int port = 5001;

	// 1st arg is the server IP
	if(argc > 1) {
		std::istringstream ss(argv[1]);
		ss >> address;
	}

	Client client(address, port);

	while(1) {
		std::cout << "command: ";
		std::string command;
		std::cin >> command;

		std::cout << "sent command " << command << (client.command(command) ? " succesfully\n" : " unsuccessfully\n");

		std::cout << "response: " << client.receive() << std::endl;
	}

	return 0;
}
