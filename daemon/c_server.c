#include "c_server.h"

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

connection_t *create_connection(server_t* server, int connection_fd) {
	if(server->connections == NULL) {
		server->connections = (connection_t*)malloc(sizeof(connection_t));
		server->connections_n = 1;
	} else {
		++(server->connections_n);
		server->connections = realloc(server->connections, server->connections_n * sizeof(connection_t));
	}

	connection_t *cur_con = NULL;
	if(server->connections != NULL)
	{
		cur_con = server->connections + (server->connections_n-1);
		cur_con->conn_fd = connection_fd;
		cur_con->buff = NULL;
		cur_con->buff_len = 0;

		cur_con->unwritten = NULL;
		cur_con->unwritten_len = 0;
	}

	return cur_con;
}

// delete descriptor from epoll and close it
int close_fd(server_t *server, int fd, const char *signature) {
	int err = epoll_ctl(server->epoll_fd, EPOLL_CTL_DEL, fd, NULL);
	if (err == -1) {
		perror("epoll_ctl");
		printf("failed to delete %s socket to epoll event\n", signature);
		return err;
	}

	err = close(fd);
	if (err == -1) {
		perror("close");
		printf("failed to close %s socket\n", signature);
		return err;
	}

	return 0;
}

// routine for adding O_NONBLOCK flag to the open descriptor
int socket_nonblocking(int socket)
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

	// socket creation
	err = (server->listen_fd = socket(AF_INET, SOCK_STREAM, 0));
	if (err == -1) {
		perror("socket");
		printf("Failed to create socket endpoint\n");
		return err;
	}

	// socket binding
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

	// make it non-blocking
	err = socket_nonblocking(server->listen_fd);
	if (err) {
		printf("failed to make server socket nonblocking\n");
		return err;
	}

	// start listening
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
			server->epoll_fd, events, sizeof(events), -1);
		if (fds_len == -1) {
			if (errno == EINTR) {
				return 0;
			}

			perror("epoll_wait");
			printf("failed to wait for epoll events");
			return -1;
		}

		for (int i = 0; i < fds_len; i++) {

			// when the connection hangs up from error or, peer side
			// carry on waiting for next events
			if ((events[i].events & (EPOLLERR | EPOLLHUP)) ||
				(!(events[i].events & EPOLLIN))) {

				if(events[i].events & EPOLLHUP ) {

					connection_t* con = events[i].data.ptr;
					if(con != NULL) {
						err = close_fd(server, con->conn_fd, "peer connection");
					}

				}

				continue;
			}

			// if the event is coming from the listening socket
			if (events[i].data.fd == server->listen_fd) {
				socklen_t client_len;
				struct sockaddr_in client_addr;
				client_len = sizeof(client_addr);

				connection_t *cur_con = create_connection(
					server,
					accept(server->listen_fd,
						   (struct sockaddr *) &client_addr, &client_len)
				);

				if(cur_con == NULL) {
					perror("allocating memory failed");
					return -2;
				}

				if (cur_con->conn_fd == -1) {
					if ((errno != EAGAIN) && (errno != EWOULDBLOCK)) {
						perror("accept");
						return errno;
					} else {
						continue;
					}
				}

				int err;
				if((err = socket_nonblocking(cur_con->conn_fd))) {
					perror("connection blocking");
					return err;
				}

				event.events = EPOLLIN | EPOLLET;
				event.data.fd = cur_con->conn_fd;
				event.data.ptr = cur_con;
				if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, cur_con->conn_fd,
							  &event) == -1) {
					perror("epoll_ctl: conn_sock");
					return errno;
				}
				// if the event is coming from the connected socket
			} else {
				server->connection_callback(events[i].data.ptr);
			}
		}
	}

	return 0;
}


int server_close(server_t *server) {
	int err = 0;

	err = close_fd(server, server->listen_fd, "listen socket");

	for(int i=0;i<server->connections_n;++i) {
		connection_t *conn = server->connections + i;
		if(conn != NULL) {
			char signature[22]; // connection + ' ' + max size of integer digits
			memset(signature, '\0', 22);
			sprintf(signature, "connection %d", i);
			err = close_fd(server, conn->conn_fd, signature);
		}
		if(conn->buff_len) {
			conn->buff_len = 0;
			if(conn->buff) {
				free(conn->buff);
				conn->buff = NULL;
			}
		}
		if(conn->unwritten_len) {
			conn->unwritten_len = 0;
			if(conn->unwritten) {
				free(conn->unwritten);
				conn->unwritten = NULL;
			}
		}
	}

	server->connections_n = 0;
	free(server->connections);

	err = close(server->epoll_fd);
	if (err == -1) {
		perror("close");
		printf("failed to close epoll socket");
		return err;
	}

	return err;
}
