#include "c_io.h"
#include "c_server.h"


#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

/**
 * server worker
 */
void loop_server();

/**
 * connections' handler
 */
int conn_handler(int fd);


static server_t server;

int main(int nchar, char *schar[]) {

	pid_t pid;

	server.epoll_fd = -1;
	server.conn_fd = -1;
	server.listen_fd = -1;
	server.connection_callback = conn_handler;

	pid = fork();
	if(pid == 0)
	{
		loop_server();
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
