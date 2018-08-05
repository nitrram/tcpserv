#include "cpp_server.h"

#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/timerfd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>

#include <iostream>

namespace tcpserv {

	int Server::buf_len = 1024;

	Server::Server() :
		m_Epoll_fd(-1),
		m_Listen_fd(-1) {
	}

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
					if(events[i].events & EPOLLHUP ) {
						Connection *con = static_cast<Connection*>(events[i].data.ptr);
						if(con != NULL) {
							err = closeSocketDescriptor(con->getSocketDescriptor(), "peer connection");
						}
					}
					continue;
				}

				// if the event is coming from the listening socket
				if (events[i].data.fd == m_Listen_fd) {
					socklen_t client_len;
					struct sockaddr_in client_addr;
					client_len = sizeof(client_addr);

					SConnection cur_con =
						std::make_shared<Connection>(
							accept(m_Listen_fd,
								   reinterpret_cast<struct sockaddr *>(&client_addr), &client_len)
						);
					m_Connections.push_back(cur_con);

					if (cur_con->getSocketDescriptor() == -1) {
						if ((errno != EAGAIN) && (errno != EWOULDBLOCK)) {
							std::cerr << "accept\n";
							return;
						} else {
							continue;
						}
					}

					int err;
					if((err = makeSockNonBlocking(cur_con->getSocketDescriptor()))) {
						std::cerr << "connection blocking\n";
						return;
					}

					event.events = EPOLLIN | EPOLLET;
					event.data.ptr = cur_con.get();
					if (epoll_ctl(m_Epoll_fd, EPOLL_CTL_ADD, cur_con->getSocketDescriptor(),
								  &event) == -1) {
						std::cerr << "epoll_ctl: conn_sock\n";
						return;
					}
				// if the event is coming from the connected socket
				} else {
					// send back data + timer_fd for optional opration
					Connection *conn = static_cast<tcpserv::Connection*>(events[i].data.ptr);
					if(conn->getTimerFd() != -1) {
						conn->setTimerFd(-1);
						conn->callStage(conn);
					} else {
						if(m_Connection_callback(events[i].data.ptr) == START_TIMER) {
							conn->setTimerFd(timerfd_create(CLOCK_REALTIME, 0));
							if (conn->getTimerFd() == -1) {
								std::cerr << "timerfd_create\n";
								return;
							}

							struct timespec now;
							if (clock_gettime(CLOCK_REALTIME, &now) == -1) {
								std::cerr << "clock_gettime\n";
								return;
							}

							struct itimerspec timer_val;
							timer_val.it_value.tv_sec = now.tv_sec + 1;
							timer_val.it_value.tv_nsec = now.tv_nsec;
							timer_val.it_interval.tv_sec = 0;
							timer_val.it_interval.tv_nsec = 0;

							if (timerfd_settime(conn->getTimerFd(), TFD_TIMER_ABSTIME, &timer_val, NULL) == -1) {
								std::cerr << "timerfd_settime\n";
								return;
							}
							event.events = EPOLLIN | EPOLLONESHOT;
							event.data.ptr = events[i].data.ptr;
							if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn->getTimerFd(),
										  &event) == -1) {
								std::cerr << "epoll_ctl: conn_sock\n";
								return;
							}
						} // if callback == START_TIMER
					} // if conn->getTimer() != -1
				}
			} // for(fds_len)
		} //for indefinite
	}

	void Server::finish() {
		int err = 0;
		err = closeSocketDescriptor(m_Listen_fd, "listen socket");

		for(SConnection conn : m_Connections) {
			if(conn != NULL) {
				err = closeSocketDescriptor(conn->getSocketDescriptor(), "connection");
				if(!err) {
					conn.reset();
				}
			}
			conn.reset();
		}

		err = close(m_Epoll_fd);
		if (err == -1) {
			std::cerr << "close: " << errno << std::endl;
			std::cerr << "failed to close epoll socket\n";
		}
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

	int Server::closeSocketDescriptor(int fd, std::string &&tag) {

		int err = epoll_ctl(m_Epoll_fd, EPOLL_CTL_DEL, fd, NULL);
		if (err == -1) {
			std::cerr << "epoll_ctl: " << errno << std::endl;
			std::cerr << "failed to delete " << tag << " socket to epoll event\n";
			return err;
		}

		err = close(fd);
		if (err == -1) {
			std::cerr << "failed to close " << tag << std::endl;
			return err;
		}

		return 0;
	}
}
