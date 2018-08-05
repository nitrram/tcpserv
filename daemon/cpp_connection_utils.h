#pragma once

#include <string>

namespace tcpserv {

	class Connection;

	class ConnectionHelper {
	public:
		ConnectionHelper(tcpserv::Connection *connection);

		int read();

		int write(const std::string &response);

	private:

		tcpserv::Connection * const m_Connection;

	};

}
