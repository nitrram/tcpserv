#include "c_io.h"
#include "c_server.h"


#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

/**
 * server worker
 */
void loop_server();

/**
 * connections' handler
 */
int conn_handler(int fd);

void sign_handler(int signo);

static server_t server;

int main(int nchar, char *schar[]) {

	pid_t pid;

	if (signal(SIGINT, sign_handler) == SIG_ERR ||
		signal(SIGTERM, sign_handler) == SIG_ERR) {
		perror("signal");
		printf("failed to register signal handler\n");
		return 1;
	}

	server.epoll_fd = -1;
	server.connections_n = 0;
	server.connections = NULL;
	server.listen_fd = -1;
	server.connection_callback = conn_handler;

	pid = fork();
	if(pid == 0)
	{
		loop_server();
	} else if(pid < 0) {
		perror("fork");
		printf("failed to fork the process");
		return errno;
	}
	else
	{
		char mem[20],cpu[20];
		print_cpu_stats(cpu, sizeof(cpu));
		print_mem_stats(mem, sizeof(mem));

		printf("mem: %s", mem);
		printf("cpu: %s", cpu);
	}
	return 0;
}

void loop_server() {

	server_listen(&server);
	server_work(&server);
	server_close(&server);
}

int conn_handler(int fd)
{
	int	 n = 0;
	char buf[1024];

	// clean up the buffer
	memset(buf, '\0', sizeof(buf));

	while(1) {
		n = read(fd, buf, sizeof(buf));
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

	int response_len = 0;
	char response[20];

	if(strcmp(buf, "mem") == 0) {
		response_len = print_mem_stats(response, sizeof(response));
	} else if(strcmp(buf, "cpu") == 0) {
		response_len = print_cpu_stats(response, sizeof(response));
	} else {
		strcpy(response, "incorrect command\n");
		response_len = 18;
	}

	// send client the response
	n = write(fd, response, response_len);
	if (n == -1) {
		perror("write");
		printf("failed to write to client\n");
		return -1;
	}

	return 0;
}

void sign_handler(int signo __attribute__ ((unused)))
{
	int err = server_close(&server);
	if (err) {
		perror("server_close");
		printf("failed to close the server\n");
		exit(err);
	}
}
