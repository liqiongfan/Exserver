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

#ifdef __linux__
	struct epoll_event ev, events[EPOLL_KQUEUE_NUMBER];
	memset(&ev, 0, sizeof(struct epoll_event));
	epoll_fd = epoll_create(1);
#else
	struct kevent ev, events[100];
	struct timespec ts;
	memset(&ev, 0, sizeof(struct kevent));
	epoll_fd = kqueue();
#endif
	assert(epoll_fd != -1);
	char *request_url = NULL, *request_method = NULL;

#ifdef __linux__
	ev.data.fd = read_fd;
	ev.events  = EPOLLIN;
	result = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, read_fd, &ev);
#else
	ts.tv_sec = ts.tv_nsec = 0;
	EV_SET(&ev, read_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
	result = kevent(epoll_fd, &ev, 1, NULL, 0, &ts);
#endif
	assert(  result != -1 );

	while ( true )
	{
		memset(events, 0, sizeof(events));
#ifdef __linux__
		epoll_num = epoll_wait(epoll_fd, events, EPOLL_KQUEUE_NUMBER, -1);
#else
		epoll_num = kevent(epoll_fd, NULL, 0, events, EPOLL_KQUEUE_NUMBER, NULL);
#endif
		if ( epoll_num )
		{
			for (epoll_index = 0; epoll_index < epoll_num; ++epoll_index)
			{
#ifdef __linux__
				if ( events[epoll_index].events & ( EPOLLRDHUP ) )
#else
				if ( events[epoll_index].flags == EV_EOF || events[epoll_index].flags == EV_ERROR )
#endif
				{
#ifdef __linux__
					close(events[epoll_index].data.fd);
#else
					close(events[epoll_index].ident);
#endif
#ifdef __linux__
					epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[epoll_index].data.fd, NULL);
#else
					memset(&ev, 0, sizeof(struct kevent));
					ts.tv_sec = ts.tv_nsec = 0;
					EV_SET(&ev, events[epoll_index].ident, EVFILT_READ, EV_DELETE, 0, 0, NULL);
					kevent(epoll_fd, &ev, 1, NULL, 0, &ts);
#endif
				}
#ifdef __linux__
				if ( events[epoll_index].events & ( EPOLLIN | EPOLLOUT | EPOLLHUP | EPOLLERR ) )
#else
				if ( events[epoll_index].filter == EVFILT_READ )
#endif
				{
#ifdef __linux__
					if ( events[epoll_index].data.fd == read_fd )
#else
					if ( events[epoll_index].ident == read_fd )
#endif
					{
						new_client_fd = socket_recv_fd(read_fd);
#ifdef __linux__
						memset(&ev, 0, sizeof(struct epoll_event));
#else
						memset(&ev, 0, sizeof(struct kevent));
#endif
						socket_nonblock(new_client_fd);
#ifdef __linux__
						ev.data.fd = new_client_fd;
						ev.events  = EPOLLIN;
						result = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_client_fd, &ev);
						if ( result == -1 && ( errno == EEXIST || errno == EINTR ) ) continue;
#else
						memset(&ev, 0, sizeof(struct kevent));
						ts.tv_sec = ts.tv_nsec = 0;
						EV_SET(&ev, new_client_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
						result = kevent(epoll_fd, &ev, 1, NULL, 0, &ts);
						if ( result == -1 ) continue;
#endif
					}
					else
					{
#ifdef __linux__
						new_client_fd = events[epoll_index].data.fd;
#else
						new_client_fd = events[epoll_index].ident;
#endif
						/* Read data from fd */
						char *buff = get_socket_stream_data(new_client_fd, &stream_length);

						if ( buff )
						{
							/* if data ok */
							start_pos = 0;
							while (true)
							{
								EXLIST *stream = parse_http_stream(buff, &start_pos);

								EXLIST_FOREACH(stream, ptr)
								{
									EX_HTTP_HEADER *header = ELV_VALUE_P(ptr);

									/* Find the request_method */
									if (strncmp(HEADER_KEY_P(header), HT_REQUEST_METHOD) == 0) {
										request_method = HEADER_VALUE_P(header);
									}

									/* Find request_uri */
									if (strncmp(HEADER_KEY_P(header), HT_REQUEST_URL) == 0) {
										request_url = HEADER_VALUE_P(header);
									}

									/* Find http version */
									if (strncasecmp(HEADER_KEY_P(header), HT_HTTP_VERSION) == 0) {
										if (strncasecmp(HEADER_VALUE_P(header), HT_HTTP_1_0) == 0) {
											http_protocol = 0;
										} else if (strncasecmp(HEADER_VALUE_P(header), HT_HTTP_1_1) == 0) {
											http_protocol = 1;
											keepalive = 1;
										} else {
											http_protocol = 2;
										}
									}

									/* Find the connection header
									 * if value is keep-alive and http*/
									if (strncasecmp(HEADER_KEY_P(header), HT_CONNECTION) == 0) {
										if (strncasecmp(HEADER_VALUE_P(header), HT_KEEP_ALIVE) == 0) {
											keepalive = 1;
										} else {
											keepalive = 0;
										}
									}
								} EXLIST_FOREACH_END();

								/* after parsing steps, calling the callback */
								callback(new_client_fd, stream, request_method, request_url, keepalive);

								/* after using, destroy the http stream result. */
								destroy_exlist(stream);
								if (start_pos == stream_length) break;
							}

							/* free the stream buff */
							free(buff);

							/* if not keep alive the connection shutdown other than keep-alive */
							if (!keepalive)
							{
								shutdown(new_client_fd, SHUT_WR);
#ifndef __linux__
								memset(&ev, 0, sizeof(struct kevent));
								ts.tv_sec = ts.tv_nsec = 0;
								EV_SET(&ev, new_client_fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
								kevent(epoll_fd, &ev, 1, NULL, 0, &ts);
#endif
							} /* end if keepalive */

							/* after each setting */
							keepalive = 0;
						}
						else
						{
							free(buff);
							close(new_client_fd);
						}
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
	int epoll_num, epoll_fd, epoll_index,
		dispatch_id, client_fd, result, dispatch_index = 0;;

#ifdef __linux__
	struct epoll_event ev, event[EPOLL_KQUEUE_NUMBER];
	memset(&ev, 0 , sizeof(struct epoll_event));
#else
	struct kevent ev, event[EPOLL_KQUEUE_NUMBER];
	struct timespec ts;
	memset(&ev, 0, sizeof(struct kevent));
#endif

#ifdef __linux__
	epoll_fd = epoll_create(1);
	assert( epoll_fd != -1 );
	ev.data.fd = server_fd;
	ev.events  = EPOLLIN;
	result = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev);
#else
	epoll_fd = kqueue();
	assert( epoll_fd != -1 );
	ts.tv_sec = ts.tv_nsec = 0;
	EV_SET(&ev, server_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
	result = kevent(epoll_fd, &ev, 1, NULL, 0, &ts);
#endif
	assert( result != -1);

	while ( true )
	{
		memset(event, 0, sizeof(event));

#ifdef __linux__
		epoll_num = epoll_wait(epoll_fd, event, EPOLL_KQUEUE_NUMBER, -1);
#else
		epoll_num = kevent(epoll_fd, NULL, 0, event, EPOLL_KQUEUE_NUMBER, NULL);
#endif
		if ( epoll_num )
		{
			for (epoll_index = 0; epoll_index < epoll_num; ++epoll_index)
			{
#ifdef __linux__
				if ( event[epoll_index].events & EPOLLIN )
#else
				if ( event[epoll_index].filter & EVFILT_READ )
#endif
				{
					client_fd = accept(server_fd, NULL, NULL);
					if ( client_fd )
					{
						/* if current request get the boundary of 1 million
						 * reset it to zero and restart from zero */
						if ( dispatch_index == 1000000 ) dispatch_index = 0;

						/* Dispatch to dispatch_id */
						dispatch_id = dispatch_index++ % finally_worker_num;
						/*printf("client_id: %d dispatch to : %d\n", client_fd, dispatch_id);*/

						/* Send the file descriptor to the worker process
						 * after sending closing it in the master process
						 * to keep the fd in the child process can be closed  successfully */
						socket_send_fd(fd_sockets[dispatch_id][0], client_fd);
						close(client_fd);
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

#ifdef __linux__
	/* Using the epoll API */
	epoll_fd = epoll_create(1);
#else
	epoll_fd = kqueue();
#endif

#ifdef __linux__
	struct epoll_event ev, events[100];
	memset(&ev, 0, sizeof(struct epoll_event));
	memset(events, 0, sizeof(struct epoll_event) * 100);

	ev.events = EPOLLIN;
	ev.data.fd = server_fd;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev);

#else
	struct kevent ev, events[100];
	struct timespec ts;
	memset(&ev, 0, sizeof(struct kevent));
	memset(events, 0, sizeof(struct kevent) * 100);

	ts.tv_sec = ts.tv_nsec = 0;
	EV_SET(&ev, server_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
	kevent(epoll_fd, &ev, 1, NULL, 0, &ts);
#endif

	while ( true )
	{
#ifdef __linux__
		epoll_num = epoll_wait(epoll_fd, events, 100, -1);
#else
		epoll_num = kevent(epoll_fd, NULL, 0, events, 100, NULL);
#endif
		if ( epoll_num )
		{
			for (_j = 0; _j < epoll_num; ++_j)
			{
				keepalive = 0;

#ifdef __linux__
				if ( events[_j].events & EPOLLRDHUP ) close(events[_j].data.fd);
				if ( events[_j].events & ( EPOLLIN | EPOLLOUT | EPOLLHUP | EPOLLERR ) )
#else
				if ( events[_j].flags == EV_ERROR || events[_j].flags == EV_EOF ) close(events[_j].ident);
#endif
				{
#ifdef __linux__
					if ( events[_j].data.fd == server_fd )
#else
					if ( events[_j].ident == server_fd )
#endif
					{
						client_fd = accept(server_fd, NULL, NULL);
						if ( client_fd )
						{
							socket_nonblock(client_fd);
#ifdef __linux__
							memset(&ev, 0, sizeof(struct epoll_event));
							ev.data.fd = client_fd;
							ev.events = EPOLLIN;
							epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
#else
							memset(&ev, 0, sizeof(struct kevent));
							ts.tv_sec = ts.tv_nsec = 0;
							EV_SET(&ev, client_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
							kevent(epoll_fd, &ev, 1, NULL, 0, &ts);
#endif
						}
					}
					else
					{
#ifdef __linux__
						client_fd = events[_j].data.fd;
#else
						client_fd = events[_j].ident;
#endif

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
											keepalive = 1;
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
