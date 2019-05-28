/**
 * File: http_server.c for project sockets
 * Email: xeapplee@gmail.com
 * website: https://www.supjos.cn
 * Author: Josin 
 * Copyright (c) Exserver-2019 all rights reserved
 */

#include "../kernel/exlist.h"
#include "../kernel/types.h"
#include "../kernel/net/socket.h"

#include <sys/epoll.h>

int main()
{
	size_t start_pos = 0, stream_length;
	ssize_t read_num = 0, read_all_num = 0;
	int _result, server_fd = socket_create(AF_INET, SOCK_STREAM, 0);

	socket_bind_address(server_fd, "0.0.0.0", 8181);

	_result = listen(server_fd, 100);
	assert(_result != -1);

	EXLIST_V *ptr;
	int client_fd, times = 1;

again_connect:
	client_fd = accept(server_fd, NULL, NULL);
	socket_nonblock(client_fd);

	printf("client fd: %d\n", client_fd);
	while (1)
	{
		char *buff = get_socket_stream_data(client_fd, &stream_length);

		if ( buff )
		{
			while ( true )
			{
				EXLIST *stream = parse_http_stream( buff, &start_pos );

				EXLIST_FOREACH(stream, ptr){
					EX_HTTP_HEADER *header = ELV_VALUE_P(ptr);
					printf("%s:%s\n", HEADER_KEY_P(header), HEADER_VALUE_P(header));
				} EXLIST_FOREACH_END();

				destroy_exlist(stream);
				if ( start_pos == stream_length ) break;
			}

			start_pos = 0;

			free(buff);

			char *response = generate_response_string(
					200, "OK", "hello world",
					4,
					"Connection: keep-alive",
					"Content-Type: text/html",
					"Content-Length: 11",
					"Status Code: 200"
			);
			write(client_fd, response, strlen(response));
			free(response);
		}
		else
		{
			close(client_fd);
			goto again_connect;
		}
	}
}