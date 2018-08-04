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
int conn_handler(void *data);

void sign_handler(int signo);

static size_t inflateBuffer(char **dst, size_t dst_len, const char *src, size_t extra);

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

int custom_handler(void *data) {

	connection_t *connection = data;

	int response_len = 0;
	char response[20];

	response_len = print_cpu_stage1(connection->custom_map, response, sizeof(response));

	// send client the response
	while(1) {
		int n = write(connection->conn_fd, response, response_len);
		if (n == -1) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				continue;
			}
			perror("write");
			printf("failed to write to client\n");
			return -1;
		} else if(n < response_len) {
			connection->unwritten_len = inflateBuffer(
				&connection->unwritten, connection->unwritten_len, response+n, response_len - n);
		}

		break;
	}

	return 0;
}

int conn_handler(void *data)
{
	int	 n = 0;
	char buf[TCPSERV_BUF_LEN];
	connection_t *connection = data;

	// clean up the buffer
	memset(buf, '\0', sizeof(buf));

	if(connection->buff_len) {
		connection->buff_len = 0;
		free(connection->buff);
		connection->buff = NULL;
	}


	while(1) {
		n = read(connection->conn_fd, buf, sizeof(buf));
		if (n == -1) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				return errno; // bail out to the waiting loop
			}

			perror("read");
			printf("failed to read from the client\n");
			return -1;
		}

		if (n == 0) {
			break;
		}

		connection->buff_len = inflateBuffer(
			&connection->buff, connection->buff_len, buf, n);

		if(connection->buff[n-1] == 0xa) // new line means end of command
			break;
	}

	//printf("command: %s %ld\n", connection->buff, connection->buff_len);
	int response_len = 0;
	char response[20];

	if(connection->buff && strcmp(connection->buff, "mem\n") == 0) {
		response_len = print_mem_stats(response, sizeof(response));
	} else if(connection->buff && strcmp(connection->buff, "cpu\n") == 0) {
		// get current stats and let timer do the rest -> on timer
		// is finished, redirect to the other callback
		print_cpu_stage0(connection->custom_map);
		connection->custom_one_shot = custom_handler;
		return START_TIMER;

	} else {
		strcpy(response, "incorrect command\n");
		response_len = 18;
	}


	// send client the response
	while(1) {
		n = write(connection->conn_fd, response, response_len);
		if (n == -1) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				continue;
			}
			perror("write");
			printf("failed to write to client\n");
			return -1;
		} else if(n < response_len) {
			connection->unwritten_len = inflateBuffer(
				&connection->unwritten, connection->unwritten_len, response+n, response_len - n);
		}

		break;
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

size_t inflateBuffer(char **dst, size_t dst_len, const char *src, size_t extra) {

	if(dst_len == 0) {
		if(*dst != NULL) {
			free(*dst);
		}
		*dst = (char*)calloc(extra+1, sizeof(char));
		memcpy(*dst, src, extra);
		return extra;
	} else
	{
		*dst = (char*)realloc(*dst, (dst_len+extra)*sizeof(char));
		memcpy(*dst+dst_len, src, extra);
		return dst_len + extra;
	}
}
