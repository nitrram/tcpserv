#pragma once

#ifdef __cplusplus
extern "C" {
#endif


typedef int (*connection_handler)(int);

/** TODO make it dynamic in the terms of open connections
 *
 */
typedef struct server {
	int epoll_fd;
	int listen_fd;
	int conn_fd;
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


#ifdef __cplusplus
}
#endif
