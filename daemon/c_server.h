#pragma once

#ifdef __cplusplus
extern "C" {
#endif


typedef int (*connection_handler)(int);

/**
 * Encapsulates the properties of the server.
 */
typedef struct server {
	int epoll_fd;
	int listen_fd;
	int conn_fd;
	connection_handler connection_callback;

} server_t;


/**
 * Creates a socket for the server and makes it passive such that
 * we can wait for connections on it later.
 */
int server_listen(server_t* server);


	/**
 * Accepts connections and processes them using the handler specfied
 * in the server struct.
 */
int server_work(server_t* server);


#ifdef __cplusplus
}
#endif
