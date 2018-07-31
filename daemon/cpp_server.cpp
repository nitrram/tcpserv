#include "cpp_server.h"

#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>

#include <iostream>

namespace tcpserv {

	Server::Server() :
		m_Epoll_fd(-1),
		m_Listen_fd(-1),
		m_Conn_fd(-1) {}

	int Server::setup(THandler handler) {
		int err = 0;

		// set callback
		m_Connection_callback = handler;

		// socket creation
		err = (m_Listen_fd = socket(AF_INET, SOCK_STREAM, 0));
		if (err == -1) {
			std::cerr << "socket: " << "Failed to create socket endpoint\n";
			return err;
		}

		// socket binding
		struct sockaddr_in server_addr = { 0 };
		server_addr.sin_family = AF_INET;
		server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		server_addr.sin_port = htons(5001);
		err = bind(m_Listen_fd,
				   reinterpret_cast<struct sockaddr*>(&server_addr),
				   sizeof(server_addr));
		if (err == -1) {
			std::cerr << "bind: " << "Failed to bind socket to address\n";
			return err;
		}

		// make it non-blocking
		err = makeSockNonBlocking(m_Listen_fd);
		if (err) {
			std::cerr << "non-block: " << "Failed to make server socket nonblocking\n";
			return err;
		}

		// start listening
		err = listen(m_Listen_fd, 5);
		if (err == -1) {
			std::cerr << "listen: " << "Failed to put socket in passive mode\n";
			return err;
		}

		return err;
	}


	void Server::loop() {
		int epoll_fd;
		int err = 0;

		// creates a new epoll instance and returns a file
		// descriptor referring to that instance.
		err = (epoll_fd = epoll_create1(0));
		if (err == -1) {
			std::cerr << "epoll_create: " << "Couldn't create epoll fd\n";
			return;
		}

		m_Epoll_fd = epoll_fd;

		struct epoll_event event = { 0 };
		struct epoll_event events[4];

		// linked to epoll_fd.
		event.data.fd = m_Listen_fd;
		event.events   = EPOLLIN | EPOLLET;

		err = epoll_ctl(m_Epoll_fd, EPOLL_CTL_ADD, m_Listen_fd, &event);
		if (err == -1) {
			std::cerr << "epoll_ctl: " << "Failed to add listen socket to epoll event\n";
			return;
		}

		for (;;) {
			int fds_len = epoll_wait(
				m_Epoll_fd, events, 64, -1);
			if (fds_len == -1) {
				if (errno == EINTR) {
					return;
				}

				std::cerr << "epoll_wait: " << "Failed to wait for epoll events\n";
				return;
			}

			for (int i = 0; i < fds_len; i++) {

				// when the connection hangs up from error or, peer side
				// carry on waiting for next events
				if ((events[i].events & (EPOLLERR | EPOLLHUP)) ||
					(!(events[i].events & EPOLLIN))) {
					continue;
				}

				// if the event is coming from the listening socket
				if (events[i].data.fd == m_Listen_fd) {
					socklen_t client_len;
					struct sockaddr_in client_addr;
					client_len = sizeof(client_addr);
					m_Conn_fd = accept(m_Listen_fd,
									   reinterpret_cast<struct sockaddr *>(&client_addr), &client_len);
					if (m_Conn_fd == -1) {
						if ((errno != EAGAIN) && (errno != EWOULDBLOCK)) {
							std::cerr << "accept\n";
							return;
						} else {
							continue;
						}
					}

					int err;
					if((err = makeSockNonBlocking(m_Conn_fd))) {
						std::cerr << "connection blocking\n";
						return;
					}

					event.events = EPOLLIN | EPOLLET;
					event.data.fd = m_Conn_fd;
					if (epoll_ctl(m_Epoll_fd, EPOLL_CTL_ADD, m_Conn_fd,
								  &event) == -1) {
						std::cerr << "epoll_ctl: conn_sock\n";
						return;
					}
					// if the event is coming from the connected socket
				} else {
					m_Connection_callback(events[i].data.fd);
				}
			} // for(fds_len)
		} //for indefinite
	}

	int Server::makeSockNonBlocking(int socket) {
		int err = 0;
		int flags;

		err = (flags = fcntl(socket, F_GETFL, 0));
		if (err == -1) {
			std::cerr << "fcntl: " << "Failed to retreive socket flags\n";
			return -1;
		}

		flags |= O_NONBLOCK;

		err = fcntl(socket, F_SETFL, flags);
		if (err == -1) {
			std::cerr << "fcntl: " << "Failed to set socket flags\n";
			return -1;
		}

		return 0;
	}
}
