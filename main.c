#include <stdio.h>

#include "kernel/net/http_socket.h"
#include "kernel/exlist.h"
#include "kernel/types.h"
#include "kernel/net/socket.h"

#include <sys/socket.h>

void server_callback(int fd, EXLIST *header, char *request_method, char *request_url, int keep_alive)
{
	char *response;
	if ( keep_alive )
		response = generate_response_string(200, "OK", "World", 3, "Content-Type: text/html", "Content-Length: 5", "Connection: keep-alive");
	else
		response = generate_response_string(200, "OK", "hello", 3, "Content-Type: text/html", "Content-Length: 5", "Connection: close");
	write(fd, response, strlen(response));
	free(response);
}

int main(int argc, char *argv[])
{
	int server_fd;

	server_fd = http_server_init("0.0.0.0", 8181, 1000);
	generate_worker(3, server_callback);
	master_process(server_fd);


	return 0;
}