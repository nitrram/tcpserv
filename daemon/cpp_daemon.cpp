#include "cpp_server.h"
#include "cpp_connection.h"
#include "cpp_connection_utils.h"
#include "cpp_io.h"


#include <signal.h>

#include <unistd.h>
#include <iostream>
#include <cstring>

void sign_handler(int signo);

static std::shared_ptr<tcpserv::Server> server = std::make_shared<tcpserv::Server>();

int main(int argc, char *argv[]) {

	if (signal(SIGINT, sign_handler) == SIG_ERR ||
		signal(SIGTERM, sign_handler) == SIG_ERR) {
		std::cerr << "failed to register signal handler\n";
		return 1;
	}

	tcpserv::THandler stagedCallback =
		[](void *data) {
			tcpserv::Connection *connection = static_cast<tcpserv::Connection*>(data);
			tcpserv::ConnectionHelper helper(connection);

			std::string response = tcpserv::Printer::getCpuStatsStaged(connection->getStagedData());

			return helper.write(response);
		};

	tcpserv::THandler callback =
		[stagedCallback](void *data) {
			tcpserv::Connection *connection = static_cast<tcpserv::Connection*>(data);
			tcpserv::ConnectionHelper helper(connection);

			int err;
			if((err = helper.read())) {
				std::cerr << "read from socket error\n";
				return err;
			}

			std::string response;
			std::string cmd(connection->getBuffer().begin(), connection->getBuffer().end());

			if(cmd == "mem\n") {
				response = tcpserv::Printer::getMemStats();
			} else if(cmd == "cpu\n") {
				connection->setStagedData(tcpserv::Printer::getCpuValues());
				connection->setStagedHandler(stagedCallback);
				return START_TIMER;
			} else {
				response =	"incorrect command\n";
			}

			return helper.write(response);
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

void sign_handler(int signo __attribute__ ((unused))) {
	server->finish();
}
