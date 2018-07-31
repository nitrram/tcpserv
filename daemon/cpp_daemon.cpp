#include "cpp_server.h"
#include "cpp_io.h"

#include <unistd.h>
#include <pthread.h>
#include <functional>
#include <iostream>
#include <cstring>

int main(int argc, char *argv[]) {

	tcpserv::Server server;
	tcpserv::Printer printer;

	tcpserv::THandler callback =
		[printer](int fd) {
			int	 n = 0;
			char buf[1024];

			memset(buf, '\0', 1024);

			while(1) {
				n = read(fd, buf, 1024);
				if (n == -1) {
					if (errno == EAGAIN || errno == EWOULDBLOCK) {
						break;
					}

					std::cerr << "read: " << "Failed to read from the client\n";
					return errno;
				}

				if (n == 0) {
					break;
				}
			}

			std::string sbuf(buf);
			std::string response;

			if(sbuf ==	"mem") {
				response = printer.getMemStats();
			} else if(sbuf == "cpu") {
				response = printer.getCpuStats();
			} else {
				response =	"incorrect command\n";
			}

			std::cout << response << std::endl;

			// send client the response
			n = write(fd, response.c_str(), response.size());
			if (n == -1) {
				std::cerr << "write: " << "Failed to write to client\n";
				return -1;
			}

			return 0;
		};

	int err;
	if((err = server.setup(callback)) == 0) {
		int pid = fork();
		if(pid == 0) {
			server.loop();
		}
	} else {
		std::cerr << "server.setup: " << err << std::endl;
		return -1;
	}

	return 0;
}
