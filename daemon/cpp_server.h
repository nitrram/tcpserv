#pragma once

#include <functional>

namespace tcpserv {

	using THandler = std::function<int(int)>;

	class Server {
	public:

		Server();

		int setup(THandler);

		void loop();

	private:

		static int makeSockNonBlocking(int fd);

	private:
		int m_Epoll_fd;
		int m_Listen_fd;
		int m_Conn_fd;
		THandler m_Connection_callback;
	};

}
