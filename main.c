#include <stdio.h>

#include "kernel/net/http_socket.h"
#include "kernel/exlist.h"
#include "kernel/types.h"
#include "kernel/net/socket.h"

void server_callback(int fd, EXLIST *header, char *request_method, char *request_url, int keep_alive)
{
	char *response;
	if ( keep_alive )
		response = generate_response_string(200, "OK", "<!DOCTYPE html> <html> <head> <title>Exserver 1.0</title> </head> <body> <div style=\"text-align: center\"> <h1>Hello Exserver 1.0</h1> </div> </body> </html>", 4, "Server: Exserver/1.0" ,"Content-Type: text/html", "Content-Length: 156", "Connection: keep-alive");
	else
		response = generate_response_string(200, "OK", "hello", 4, "Content-Type: text/html", "Server: Exserver/1.0" , "Content-Length: 5", "Connection: close");
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