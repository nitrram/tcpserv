#include "cpp_client.h"

#include <iostream>
#include <sstream>

static void show_usage(std::string name)
{
	std::cerr << "Usage: " << name << " <option(s)> IP_ADDRESS\n"
			  << "Options:\n"
			  << "\t-h,--help\t\tShow this help message\n"
			  << "\t-c,--connection IP_ADDRESS\tSpecify the IP address of the server"
			  << std::endl;
}


int main(int argc, char *argv[]) {

	std::string address = "127.0.0.1";
	int port = 5001;

	// process command line opts
	for (int i = 1; i < argc; ++i) {
		std::string arg = argv[i];
		if ((arg == "-h") || (arg == "--help")) {
			show_usage(argv[0]);
			return 0;
		} else if ((arg == "-c") || (arg == "--connection")) {
			if (i + 1 < argc) {
				std::istringstream iss(argv[++i]);
				iss >> address;
			} else {
				std::cerr << "--connection option requires one argument." << std::endl;
				return 1;
			}
		}
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
