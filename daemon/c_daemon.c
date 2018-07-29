#include "c_io.h"
#include "c_server.h"

#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

void loop_server();

int conn_handler(int fd);

void sig_handler(int signo);

static server_t server;

int main(int nchar, char *schar[]) {

	pid_t pid;

	server.epoll_fd = -1;
	server.listen_fd = -1;
	server.connection_callback = conn_handler;

	pid = fork();
	if(pid == 0)
	{
		loop_server();
	}
	else
	{
		print_cpu_stats(stdout);
		print_mem_stats(stdout);
	}
	return 0;
}

void loop_server() {

	server_listen(&server);
	server_work(&server);
	/*while(1) {
		usleep(200000);
		}*/
}

int conn_handler(int fd)
{
	int	 n = 0;
	char buf[1024];

	for (;;) {
		n = read(fd, buf, 1024);
		if (n == -1) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				break;
			}

			perror("read");
			printf("failed to read from the client\n");
			return -1;
		}

		if (n == 0) {
			break;
		}
	}

	//buf = command
	printf("%s\n", buf);

	char * response = "test\n";

	n = write(fd, response, strlen(response));
	if (n == -1) {
		perror("write");
		printf("failed to write to client\n");
		return -1;
	}

	return 0;
}

void sig_handler(int signo __attribute__((unused))) {
	int err;

	if (server.listen_fd != -1) {
		err = close(server.listen_fd);
		if (err == -1) {
			perror("close");
			printf("failed to close connection file descriptor\n");
			exit(err);
		}
		server.listen_fd = -1;
	}

	if (server.epoll_fd != -1) {
		err = close(server.epoll_fd);
		if (err == -1) {
			perror("close");
			printf("failed to close server epoll fd\n");
			exit(err);
		}

		server.epoll_fd = -1;
	}
}
