#include "cpp_connection_utils.h"
#include "cpp_connection.h"
#include "cpp_server.h"

#include <unistd.h>
#include <iostream>
#include <cstring>

namespace tcpserv {
	ConnectionHelper::ConnectionHelper(tcpserv::Connection * const connection) :
		m_Connection(connection) {
	}

	int ConnectionHelper::read() {
		char buf[tcpserv::Server::buf_len];

		memset(buf, '\0', sizeof(buf));

		m_Connection->resetBuffer();

		while(1)
		{
			int n = ::read(m_Connection->getSocketDescriptor(), buf, sizeof(buf));
			if (n == -1) {
				if (errno == EAGAIN || errno == EWOULDBLOCK) {
					return errno;
				}

				std::cerr << "read: " << "Failed to read from the client\n";
				return errno;
			}

			if (n == 0) {
				return 0;
			}

			std::string sbuf(buf);
			m_Connection->appendBuffer({sbuf.begin(), sbuf.end()});

			if(m_Connection->getBuffer().back() == 0xa)
				return 0;
		}

		return 0;
	}

	int ConnectionHelper::write(const std::string &response) {
		// send client the response
		while(1) {
			int n = ::write(m_Connection->getSocketDescriptor(), response.c_str(), response.size());
			if (n == -1) {
				if (errno == EAGAIN || errno == EWOULDBLOCK) {
					continue;
				}
				std::cerr << "write: " << "Failed to write to client\n";
				return -1;
			} else if(static_cast<size_t>(n) < response.size()) {
				m_Connection->appendUnwritten({response.begin() + n, response.end()});
			}

			//TODO remove to pass the loop further on (now we know that the response)
			// is short enough to be sent at once
			return 0;
		}
	}
}
