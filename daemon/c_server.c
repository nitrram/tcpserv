#include "c_server.h"

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>

// routine for adding O_NONBLOCK flag to the open descriptor
static int socket_nonblocking(int socket)
{
	int err = 0;
	int flags;

	err = (flags = fcntl(socket, F_GETFL, 0));
	if (err == -1) {
		perror("fcntl");
		printf("failed to retrieve socket flags\n");
		return -1;
	}

	flags |= O_NONBLOCK;

	err = fcntl(socket, F_SETFL, flags);
	if (err == -1) {
		perror("fcntl");
		printf("failed to set socket flags\n");
		return -1;
	}

	return 0;
}

// create, bind, listen
int server_listen(server_t* server) {
	int err = 0;

	/* socket creation */
	err = (server->listen_fd = socket(AF_INET, SOCK_STREAM, 0));
	if (err == -1) {
		perror("socket");
		printf("Failed to create socket endpoint\n");
		return err;
	}

	/* socket binding */
	struct sockaddr_in server_addr = { 0 };
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(5001);
	err = bind(server->listen_fd,
			   (struct sockaddr*)&server_addr,
			   sizeof(server_addr));
	if (err == -1) {
		perror("bind");
		printf("Failed to bind socket to address\n");
		return err;
	}

	/* make it non-blocking */
	err = socket_nonblocking(server->listen_fd);
	if (err) {
		printf("failed to make server socket nonblocking\n");
		return err;
	}

	/* start listening */
	err = listen(server->listen_fd, 5);
	if (err == -1) {
		perror("listen");
		printf("Failed to put socket in passive mode\n");
		return err;
	}

	return err;
}

int server_work(server_t* server) {
	int epoll_fd;
	int err = 0;

	struct epoll_event event = { 0 };
	struct epoll_event events[4];

	// creates a new epoll instance and returns a file
	// descriptor referring to that instance.
	err = (epoll_fd = epoll_create1(0));
	if (err == -1) {
		perror("epoll_create1");
		printf("couldn't create epoll fd\n");
		return err;
	}

	server->epoll_fd = epoll_fd;


	// linked to epoll_fd.
	event.data.fd = server->listen_fd;
	event.events   = EPOLLIN | EPOLLET;

	err =
	  epoll_ctl(server->epoll_fd, EPOLL_CTL_ADD, server->listen_fd, &event);
	if (err == -1) {
		perror("epoll_ctl");
		printf("failed to add listen socket to epoll event");
		return err;
	}

	for (;;) {
		int fds_len = epoll_wait(
		  server->epoll_fd, events, 64, -1);
		if (fds_len == -1) {
			if (errno == EINTR) {
				return 0;
			}

			perror("epoll_wait");
			printf("failed to wait for epoll events");
			return -1;
		}

		for (int i = 0; i < fds_len; i++) {
			if (events[i].data.fd == server->listen_fd) {
				socklen_t client_len;
				struct sockaddr_in client_addr;
				client_len = sizeof(client_addr);

				server->conn_fd = accept(server->listen_fd,
										 (struct sockaddr *) &client_addr, &client_len);
				if (server->conn_fd == -1) {
					if ((errno != EAGAIN) && (errno != EWOULDBLOCK)) {
						perror("accept");
						return errno;
					} else {
						continue;
					}
				}

				int err;
				if((err = socket_nonblocking(server->conn_fd))) {
					perror("connection blocking");
					return err;
				}

				event.events = EPOLLIN | EPOLLET;
				event.data.fd = server->conn_fd;
				if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server->conn_fd,
							  &event) == -1) {
					perror("epoll_ctl: conn_sock");
					return errno;
				}
			} else {
				server->connection_callback(events[i].data.fd);
			}
		}
	}

	return 0;
}
