#include "cpp_server.h"
#include "cpp_connection.h"
#include "cpp_io.h"

#include <unistd.h>
#include <functional>
#include <iostream>
#include <cstring>

int main(int argc, char *argv[]) {

	std::shared_ptr<tcpserv::Server> server = std::make_shared<tcpserv::Server>();
	tcpserv::Printer printer;

	tcpserv::THandler callback =
		[printer](void *data) {
			int	 n = 0;
			char buf[tcpserv::Server::buf_len];

			memset(buf, '\0', sizeof(buf));

			tcpserv::Connection *connection = static_cast<tcpserv::Connection*>(data);
			int fd = connection->getSocketDescriptor();

			while(1) {
				n = read(fd, buf, sizeof(buf));
				if (n == -1) {
					if (errno == EAGAIN || errno == EWOULDBLOCK) {
						return errno;
					}

					std::cerr << "read: " << "Failed to read from the client\n";
					return errno;
				}

				if (n == 0) {
					break;
				}

				std::string sbuf(buf);
				connection->appendBuffer({sbuf.begin(), sbuf.end()});

				if(connection->getBuffer().back() == 0xa)
					break;
			}

			std::string sbuf(buf);
			std::string response;

			if(sbuf ==	"mem\n") {
				response = printer.getMemStats();
			} else if(sbuf == "cpu\n") {
				response = printer.getCpuStats();
			} else {
				response =	"incorrect command\n";
			}

			while(1) {
				n = write(fd, response.c_str(), response.size());
				if (n == -1) {
					if (errno == EAGAIN || errno == EWOULDBLOCK) {
						continue;
					}
					std::cerr << "write: " << "Failed to write to client\n";
					return -1;
				} else if(static_cast<size_t>(n) < response.size()) {
					connection->appendUnwritten({response.begin() + n, response.end()});
				}

				break;
			}

			return 0;
		};

	int err;
	if((err = server->setup(callback)) == 0) {
		int pid = fork();
		if(pid == 0) {
			server->loop();
			server->finish();
		}
	} else {
		server->finish();
		std::cerr << "server.setup: " << err << std::endl;
		return -1;
	}

	return 0;
}
