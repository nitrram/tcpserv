#pragma once

#include <sys/epoll.h>
#include <stddef.h>

#define TCPSERV_BUF_LEN 1024

#ifdef __cplusplus
extern "C" {
#endif

#define START_TIMER -3

typedef int (*connection_handler)(void *);

typedef struct connection {
	int conn_fd;
	char *buff;
	size_t buff_len;

	char *unwritten;
	size_t unwritten_len;

	int timer_fd;
	int custom_map[10];
	connection_handler custom_one_shot;

} connection_t;

typedef struct server {
	int epoll_fd;
	int listen_fd;
	size_t connections_n;
	connection_t *connections;
	connection_handler connection_callback;

} server_t;


/**
 *
 */
int server_listen(server_t* server);


/**
 * Loop in the events coming from epoll
 *
 */
int server_work(server_t* server);


int server_close(server_t *server);

#ifdef __cplusplus
}
#endif

static connection_t *create_connection(server_t* server, int connection_fd);

static int close_fd(server_t *server, int fd, const char *signature);

static int socket_nonblocking(int socket);
