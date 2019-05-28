/**
 * File: http_socket.c for project sockets
 * Email: xeapplee@gmail.com
 * website: https://www.supjos.cn
 * Author: Josin 
 * Copyright (c) Exserver-2019 all rights reserved
 */

#include "../types.h"
#include "socket.h"
#include "http_socket.h"

/* Init the server socket and return the server file descriptor */
int http_server_init(const char *host, uint32_t port, int backlog)
{
	int server_fd = socket_create(AF_INET, SOCK_STREAM, 0);
	socket_bind_address(server_fd, host, port);
	socket_listen(server_fd, backlog);
	return server_fd;
}

/* Send file descriptor to the fd socket using UNIX socket
 * return -1 means error 0 means ok */
int socket_send_fd(int fd, int fd_to_send)
{
	char buff[2];
	memset(buff, 0, sizeof(buff));

	struct iovec io_vec[1];
	memset(io_vec, 0, sizeof(struct iovec));
	io_vec->iov_len = 2;
	io_vec->iov_base = buff;

	struct msghdr msg_hdr;
	memset(&msg_hdr, 0, sizeof(struct msghdr));
	msg_hdr.msg_iov     = io_vec;
	msg_hdr.msg_iovlen  = 1;
	msg_hdr.msg_name    = NULL;
	msg_hdr.msg_namelen = 0;

	struct cmsghdr cmptr;
	memset(&cmptr, 0, sizeof(struct cmsghdr));

	if ( fd_to_send < 0 )
	{
		msg_hdr.msg_control    = NULL;
		msg_hdr.msg_controllen = 0;
		buff[1] = ~fd_to_send;
		if ( buff[1] == 0 )
			buff[1] = 1; /* nonzero means failure */
	}
	else
	{
		cmptr.cmsg_level = SOL_SOCKET;
		cmptr.cmsg_type  = SCM_RIGHTS;
		cmptr.cmsg_len   = SOCKET_FD_LEN;

		msg_hdr.msg_control = &cmptr;
		msg_hdr.msg_controllen = SOCKET_FD_LEN;
		*(int *)CMSG_DATA(&cmptr) = fd_to_send;
		buff[1] = 0; /* zero means ok */
	}

	/* first char is 0 */
	buff[0] = 0;

	if ( sendmsg(fd, &msg_hdr, 0) != 2 )
		return (-1);

	return (0);
}

/* Recv file descriptor and return it using UNIX socket */
int socket_recv_fd(int fd)
{
	int  new_fd, status;
	char *ptr;
	char  buff[256];
	struct cmsghdr cmptr;
	memset(&cmptr, 0, sizeof(struct cmsghdr));
	struct iovec io_vec[1];
	memset(io_vec, 0, sizeof(struct iovec));
	struct msghdr  msg_hdr;
	memset(&msg_hdr, 0, sizeof(struct msghdr));

	io_vec[0].iov_base  = buff;
	io_vec[0].iov_len   = sizeof(buff);
	msg_hdr.msg_iov     = io_vec;
	msg_hdr.msg_iovlen  = 1;
	msg_hdr.msg_name    = NULL;
	msg_hdr.msg_namelen = 0;

	msg_hdr.msg_control    = &cmptr;
	msg_hdr.msg_controllen = SOCKET_FD_LEN;

	recvmsg(fd, &msg_hdr, 0);

	return *(int *)CMSG_DATA(&cmptr);
}

/* Worker process */
void worker_process(int index, void (*callback)(int fd, EXLIST *header, char *method, char *url, int keep_alive))
{
	int     epoll_fd,
			result,
	        epoll_num,
	        keepalive = 0,
	        epoll_index,
	        new_client_fd,
	        http_protocol = 0,
	        read_fd = fd_sockets[index][1];

	size_t start_pos, stream_length;

	struct epoll_event ev, events[100];
	memset(&ev, 0, sizeof(struct epoll_event));

	epoll_fd = epoll_create(1);
	assert(epoll_fd != -1);

	char *request_url = NULL, *request_method = NULL;

	/* fd for read */
	ev.data.fd = read_fd;
	ev.events  = EPOLLIN;
	result = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, read_fd, &ev);
	assert(  result != -1 );

	while ( true )
	{
		memset(events, 0, sizeof(events));
		epoll_num = epoll_wait(epoll_fd, events, 100, -1);
		if ( epoll_num )
		{
			for (epoll_index = 0; epoll_index < epoll_num; ++epoll_index)
			{
				if ( events[epoll_index].events & ( EPOLLRDHUP ) )
				{
					close(events[epoll_index].data.fd);
					epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[epoll_index].data.fd, NULL);
				}
				if ( events[epoll_index].events & ( EPOLLIN | EPOLLOUT | EPOLLHUP | EPOLLERR ) )
				{
					if ( events[epoll_index].data.fd == read_fd )
					{
						new_client_fd = socket_recv_fd(read_fd);
						memset(&ev, 0, sizeof(struct epoll_event));

						socket_nonblock(new_client_fd);
						ev.data.fd = new_client_fd;
						ev.events  = EPOLLIN;
						result = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_client_fd, &ev);
						if ( result == -1 )
						{
							if ( errno == EEXIST || errno == EINTR ) break;
						}
					}
					else
					{
						new_client_fd = events[epoll_index].data.fd;

						/* Read data from fd */
						char *buff = get_socket_stream_data(new_client_fd, &stream_length);

						if ( !buff )
						{
							free(buff);
							close(new_client_fd);
							break;
						}

						/* if data ok */
						start_pos = 0;
						while ( true )
						{
							EXLIST *stream = parse_http_stream( buff, &start_pos );

							EXLIST_FOREACH(stream, ptr)
							{
								EX_HTTP_HEADER *header = ELV_VALUE_P(ptr);

								/* Find the request_method */
								if ( strncmp(HEADER_KEY_P(header), "request_method", 14) == 0 )
								{
									request_method = HEADER_VALUE_P(header);
								}

								/* Find request_uri */
								if ( strncmp(HEADER_KEY_P(header), "request_uri", 11) == 0 )
								{
									request_url = HEADER_VALUE_P(header);
								}

								/* Find http version */
								if ( strncasecmp(HEADER_KEY_P(header), "http_version", 12) == 0 )
								{
									if ( strncasecmp(HEADER_VALUE_P(header), "HTTP/1.0", 8) == 0 )
									{
										http_protocol = 0;
									}
									else if ( strncasecmp(HEADER_VALUE_P(header), "HTTP/1.1", 8) == 0 )
									{
										http_protocol = 1;
									}
									else
									{
										http_protocol = 2;
									}
								}

								/* Find the connection header
								 * if value is keep-alive and http*/
								if ( strncasecmp(HEADER_KEY_P(header), "connection", 10) == 0 )
								{
									if ( strncasecmp(HEADER_VALUE_P(header), "keep-alive", 10) == 0)
									{
										keepalive = 1;
									}
									else
									{
										if ( http_protocol == 1 )
										{
											keepalive = 1;
										}
										else
											keepalive = 0;
									}
								}
							} EXLIST_FOREACH_END();

							/* after parsing steps, calling the callback */
							callback(new_client_fd, stream, request_method, request_url, keepalive);

							/* after using, destroy the http stream result. */
							destroy_exlist(stream);
							if ( start_pos == stream_length ) break;
						}
						free(buff);

						/* if not keep alive the connection shutdown other than keep-alive */
						if ( !keepalive )
						{
							shutdown(new_client_fd, SHUT_WR);
							// printf("%d delete %d result: %d errno: %d", index, new_client_fd, result, errno);
						} /* end if keepalive */
					} /* end else for newly client fd */
				} /* I/O coming */
			} /* end for */
		} /* end if epoll_num */
	} /* end while */
}

/* Generate the worker process
 * pids must have space to store the process id */
void generate_worker( int worker_num, void (*url_callback)(int fd, EXLIST *header, char *method, char *url, int keep_alive) )
{
	int _i = 0, pid = 0, result;
	finally_worker_num = worker_num;
	assert(worker_num <= MAX_WORKER_NUMBER);
	for ( ; _i < worker_num; _i++ )
	{
		result = socketpair(AF_UNIX, SOCK_STREAM, 0, fd_sockets[_i]);
		assert(result != -1);
		pid = fork();
		if ( pid == 0 )
		{
			worker_process(_i, url_callback);
		}
		else { pids[_i] = pid; }
	}
}

/* Get the master process
 * listen the newly coming client and dispatch to the other worker */
void master_process( int server_fd )
{
	long dispatch_index = 0;
	int epoll_num, epoll_fd, epoll_index, dispatch_id, client_fd, result;
	struct epoll_event ev, event[1000];
	memset(&ev, 0 , sizeof(struct epoll_event));
	memset(event, 0, sizeof(event));

	epoll_fd = epoll_create(1);
	assert( epoll_fd != -1 );

	ev.data.fd = server_fd;
	ev.events  = EPOLLIN;
	result = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev);
	assert( result != -1);


	while ( true )
	{
		epoll_num = epoll_wait(epoll_fd, event, 1000, -1);
		if ( epoll_num )
		{
			for (epoll_index = 0; epoll_index < epoll_num; ++epoll_index) {

				if ( event[epoll_index].events & EPOLLIN )
				{
					client_fd = accept(server_fd, NULL, NULL);
					if ( client_fd )
					{
						if ( dispatch_index == 10000000 ) dispatch_index = 0;
						/* Dispatch to dispatch_id */
						dispatch_id = dispatch_index % finally_worker_num;
						/*printf("client_id: %d dispatch to : %d\n", client_fd, dispatch_id);*/
						socket_send_fd(fd_sockets[dispatch_id][0], client_fd);
						close(client_fd);
						dispatch_index++;
					}
				}
			}
		}
	}
}

/* Run the http server, when coming the event kernel will invoking the callback
 * This method is one process http server which runs the main thread */
void http_server_run(int server_fd, void (*callback)(int , EXLIST *, char *, char *, int ))
{
	int _j,                 /* epoll wait sum for iterator */
		keepalive = 0,      /* keep-alive or not */
		http_protocol = 0,  /* http_protocol: 0(http/1.0) 1(http/1.1) 2(http/2.2)*/
		epoll_fd,           /* epoll file descriptor */
		epoll_num,          /* epoll num event coming number */
		client_fd;          /* client fd */

	char *request_url    = NULL,
		 *request_method = NULL;

	size_t  start_pos = 0,  /* parsing http stream start position. */
	        stream_length;  /* the http stream parsing result length, if start_pos equals to stream_length, means the parsing end. */

	/* Using the epoll API */
	epoll_fd = epoll_create(1);

	struct epoll_event ev, events[100];
	memset(events, 0, sizeof(struct epoll_event) * 100);

	memset(&ev, 0, sizeof(struct epoll_event));
	ev.events = EPOLLIN;
	ev.data.fd = server_fd;

	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev);
	while ( true )
	{
		epoll_num = epoll_wait(epoll_fd, events, 100, -1);
		if ( epoll_num )
		{
			for (_j = 0; _j < epoll_num; ++_j) {
				if ( events[_j].events & EPOLLRDHUP ) close(events[_j].data.fd);
				if ( events[_j].events & ( EPOLLIN | EPOLLOUT | EPOLLHUP | EPOLLERR ) )
				{
					if ( events[_j].data.fd == server_fd )
					{
						client_fd = accept(server_fd, NULL, NULL);
						if ( client_fd )
						{
							socket_nonblock(client_fd);
							memset(&ev, 0, sizeof(struct epoll_event));
							ev.data.fd = client_fd;
							ev.events = EPOLLIN;
							epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
						}
					}
					else
					{
						client_fd = events[_j].data.fd;

						char *buff = get_socket_stream_data(client_fd, &stream_length);

						if ( buff )
						{
							start_pos = 0;
							while ( true )
							{
								EXLIST *stream = parse_http_stream( buff, &start_pos );

								EXLIST_FOREACH(stream, ptr)
								{
									EX_HTTP_HEADER *header = ELV_VALUE_P(ptr);

									/* Find the request_method */
									if ( strncmp(HEADER_KEY_P(header), "request_method", 14) == 0 )
									{
										request_method = HEADER_VALUE_P(header);
									}

									/* Find request_uri */
									if ( strncmp(HEADER_KEY_P(header), "request_uri", 11) == 0 )
									{
										request_url = HEADER_VALUE_P(header);
									}

									/* Find http version */
									if ( strncasecmp(HEADER_KEY_P(header), "http_version", 12) == 0 )
									{
										if ( strncasecmp(HEADER_VALUE_P(header), "HTTP/1.0", 8) == 0 )
										{
											http_protocol = 0;
										}
										else if ( strncasecmp(HEADER_VALUE_P(header), "HTTP/1.1", 8) == 0 )
										{
											http_protocol = 1;
										}
										else
										{
											http_protocol = 2;
										}
									}

									/* Find the connection header
									 * if value is keep-alive and http*/
									if ( strncasecmp(HEADER_KEY_P(header), "connection", 10) == 0 )
									{
										if ( strncasecmp(HEADER_VALUE_P(header), "keep-alive", 10) == 0)
										{
											keepalive = 1;
										}
										else
										{
											if ( http_protocol == 1 )
											{
												keepalive = 1;
											}
											else
												keepalive = 0;
										}
									}
								} EXLIST_FOREACH_END();

								/* after parsing steps, calling the callback */
								callback(client_fd, stream, request_method, request_url, keepalive);

								/* after using, destroy the http stream result. */
								destroy_exlist(stream);
								if ( start_pos == stream_length ) break;
							}
							free(buff);

							/* if not keep alive the connection shutdown other than keep-alive */
							if ( !keepalive )
							{
								shutdown(client_fd, SHUT_WR);
							}
						}
						else
						{
							free(buff);
							close(client_fd);
						}
					}
				}
			}
		}
	}
}
