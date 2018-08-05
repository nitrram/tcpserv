#pragma once

#include "cpp_connection.h"
#include <functional>
#include <list>

#define START_TIMER -3

namespace tcpserv {

	using THandler = std::function<int(void *)>;

	class Server {
	public:

		Server();

		int setup(THandler);

		void loop();

		void finish();

	private:

		static int makeSockNonBlocking(int fd);

		int closeSocketDescriptor(int fd, std::string &&tag);

	public:

		static int buf_len;

	private:
		int m_Epoll_fd;
		int m_Listen_fd;
		std::list<SConnection> m_Connections;
		THandler m_Connection_callback;
	};

}
