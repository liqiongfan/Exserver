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

/* This method is the static web server function */
static void
http_static_server_callback(int fd, EX_REQUEST_T *request_t)
{
    /* The steps are the following:
     * 1: Loops the User configs to find which Host & Listening port match
     *    if not return 404 not found.
     * 2: Using sendfile to transfer data between fds. */
    
    int  _index, _index_j, is_match = 0, file_fd;
    long  server_port = 80;
    char *web_root = NULL, /* Webroot were dir store the static resources. */
         *server_host = NULL, *query_position = NULL/* Query string or not */,
         *web_index = "index.html",
         *response, /* Response body */
         *real_request_url = NULL, /* Request's URL may contains query parameters, store request url in this var not contain query para. */
         match_root[256] = {0};
    
    EXJSON *config_data = exjson_get_val_from_key(configs, "server");
    
    for (_index = 0; _index < E_NUM_P(config_data); ++_index)
    {
        /* Loop each config server */
        EXJSON *server = EV_VALUE_P(E_DATA_P(config_data) + _index);
        
        /* Loop each server's config */
        for (_index_j = 0; _index_j < E_NUM_P(server); ++_index_j)
        {
            EXJSON_V *webserver = E_DATA_P(server) + _index_j;
            if ( strncasecmp(EV_NAME_P(webserver), CONFIG_WEBROOT) == 0 )
            {
                web_root = (char *)EV_VALUE_P(webserver);
            }
            if ( strncasecmp(EV_NAME_P(webserver), CONFIG_HOST) == 0 )
            {
                server_host = (char *)EV_VALUE_P(webserver);
            }
            if ( strncasecmp(EV_NAME_P(webserver), CONFIG_LISTEN) == 0 )
            {
                server_port = *(long *)EV_VALUE_P(webserver);
            }
            if ( strncasecmp(EV_NAME_P(webserver), CONFIG_INDEX) == 0 )
            {
                web_index = (char *)EV_VALUE_P(webserver);
            }
        }
        
        /* Combine the data into match string */
        if ( server_port == 80 ) sprintf(match_root, "%s", server_host);
        else sprintf(match_root, "%s:%ld", server_host, server_port);
        
        /* See the matching webroot */
        if ( strncasecmp(request_t->server_host, match_root, strlen(match_root)) == 0 )
        {
            is_match = 1; break;
        }
    }
    
    /* Get whether host is matched or not
     * if not matched send 404 not found to client. */
    if ( !is_match ) return send_404_response(fd);
    
    /* Now real_request_url has the form like: /index.html */
    if ( request_t->request_url && *(request_t->request_url) == '/' && *(request_t->request_url + 1) == '\0')
    {
        real_request_url = web_index;
    }
    else
    {
        query_position = strchr( request_t->request_url, '?' );
        if ( query_position )
            real_request_url = exsubstr( request_t->request_url, 1, query_position - request_t->request_url, TRIM_NONE );
        else
            real_request_url = request_t->request_url + 1;
    }
    
    /* Find the file data */
    memset(match_root, 0, sizeof(match_root));
    sprintf(match_root, "%s/%s", web_root, real_request_url);
    if ( query_position ) free(real_request_url);
    
    /* Open the file */
    file_fd = open(match_root, O_RDONLY);
    if ( file_fd == -1 ) return send_404_response(fd);
    
    /* Get the file stat info. */
    struct stat file_stat;
    fstat(file_fd, &file_stat);
    
    /* Set the Content-Length according the file data */
    memset(match_root, 0, sizeof(match_root));
    sprintf(match_root, "Content-Length: %lld", file_stat.st_size);
    
    if ( request_t->keep_alive )
    {
        response = generate_response_string( 200, "OK", "", 4, "Server: Exserver/1.0", match_root, "Content-Type: text/html", "Connection: keep-alive" );
    }
    else
    {
        response = generate_response_string( 200, "OK", "", 4, "Server: Exserver/1.0", match_root, "Content-Type: text/html", "Connection: close" );
    }
    
#ifdef __linux__
    write(fd, response, strlen(response));
    sendfile(fd, file_fd, 0, file_stat.st_size);
#else
    struct iovec io_vec;
    io_vec.iov_base = response;
    io_vec.iov_len = strlen(response);
    struct sf_hdtr sf_hdtr1;
    sf_hdtr1.headers = &io_vec;
    sf_hdtr1.hdr_cnt = 1;
    sendfile( file_fd, fd, 0, ( off_t * )file_stat.st_size, &sf_hdtr1, 0 );
#endif
}

/* Worker process */
void worker_process(int index, void (*callback)(int, EX_REQUEST_T *))
{
    size_t start_pos, stream_length;
    
    EX_REQUEST_T request;
    
    int epoll_fd, result, epoll_num, epoll_index, new_client_fd, read_fd = fd_sockets[index][1];

#ifdef __linux__
	struct epoll_event ev, events[EPOLL_KQUEUE_NUMBER];
	memset(&ev, 0, sizeof(struct epoll_event));
	epoll_fd = epoll_create(1);
#else
	struct kevent ev, events[EPOLL_KQUEUE_NUMBER];
	struct timespec ts;
	memset(&ev, 0, sizeof(struct kevent));
	epoll_fd = kqueue();
#endif
	assert(epoll_fd != -1);

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
				/* before each setting */
				request.keep_alive = 0;
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
						    /* Each request need to initialise the request data */
                            memset(&request, 0, sizeof(request));
                            
							/* if data ok */
							start_pos = 0;
							while (true)
							{
								EXLIST *stream = parse_http_stream(buff, &start_pos);

								EXLIST_FOREACH(stream, ptr)
								{
									EX_HTTP_HEADER *header = ELV_VALUE_P(ptr);
         
									/* Find the HTTP Body */
                                    if ( strncasecmp(HEADER_KEY_P(header), HT_HTTP_BODY ) == 0 )
                                    {
                                        request.body = HEADER_VALUE_P(header);
                                    }
                                    
									/* Find the request host */
									if ( strncasecmp(HEADER_KEY_P(header), HT_HTTP_HOST) == 0 )
									{
									    request.server_host = HEADER_VALUE_P(header);
										/* server_host = HEADER_VALUE_P(header); */
									}

									/* Find the request_method */
									if (strncmp(HEADER_KEY_P(header), HT_REQUEST_METHOD) == 0) {
									    request.request_method = HEADER_VALUE_P(header);
										/* request_method = HEADER_VALUE_P(header); */
									}

									/* Find request_uri */
									if (strncmp(HEADER_KEY_P(header), HT_REQUEST_URL) == 0) {
									    request.request_url = HEADER_VALUE_P(header);
										/* request_url = HEADER_VALUE_P(header); */
									}

									/* Find http version */
									if (strncasecmp(HEADER_KEY_P(header), HT_HTTP_VERSION) == 0) {
										if (strncasecmp(HEADER_VALUE_P(header), HT_HTTP_1_0) == 0) {
                                            request.http_version_i = 0;
										} else if (strncasecmp(HEADER_VALUE_P(header), HT_HTTP_1_1) == 0) {
                                            request.http_version_i = 1;
                                            request.keep_alive = 1;
										} else {
                                            request.http_version_i = 2;
                                            request.keep_alive = 1;
										}
										request.http_version = HEADER_VALUE_P(header);
										request.headers = ELV_NEXT_P(ptr);
									}

									/* Find the connection header
									 * if value is keep-alive and http*/
									if (strncasecmp(HEADER_KEY_P(header), HT_CONNECTION) == 0) {
										if (strncasecmp(HEADER_VALUE_P(header), HT_KEEP_ALIVE) == 0) {
											request.keep_alive = 1;
										} else {
											request.keep_alive = 0;
										}
									}
									
								} EXLIST_FOREACH_END();

								/* after parsing steps, calling the callback */
								if ( callback == NULL )
                                    http_static_server_callback(new_client_fd, &request);
								else
                                    callback(new_client_fd, &request);

								/* after using, destroy the http stream result. */
								destroy_exlist(stream);
								if (start_pos == stream_length) break;
							}

							/* free the stream buff */
							free(buff);

							/* if not keep alive the connection shutdown other than keep-alive */
							if (!request.keep_alive)
							{
								shutdown(new_client_fd, SHUT_WR);
								close(new_client_fd);
							} /* end if keepalive */
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
void generate_worker( int worker_num, void (*url_callback)(int, EX_REQUEST_T *) )
{
	/* Read Exserver config */
	char *config_file_data = get_file_data("../kernel/config/exserver.json");
	configs = decode_json(config_file_data);
	free(config_file_data);

	/* Generate child processers */
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

/* Note that: master_process was the one port listener server
 * but static_server_start listen multi port at the same time
 * Warning: master_process mustn't be used with static_server_start at the same time. */
void static_server_start()
{
    if ( configs == NULL )
    {
        perror("Web server config file mistake!");
        exit(0);
    }
    
    int  _index, _index_j, result, epoll_fd, epoll_num, epoll_index,
         _server_fd, client_fd;
    long server_port, dispatch_index = 0, dispatch_id;
    
    /* Initialise the kqueue or epoll_fd */
#ifdef __linux__
    epoll_fd = epoll_create(1);
    struct epoll_event ev, event[EPOLL_KQUEUE_NUMBER];
#else
    struct timespec ts;
    memset(&ts, 0, sizeof(struct timespec));
    ts.tv_sec = ts.tv_nsec = 0;
    epoll_fd = kqueue();
    
    struct kevent ev, event[EPOLL_KQUEUE_NUMBER];
#endif
    assert(epoll_fd != -1);
    
    
    EXJSON *config_data = exjson_get_val_from_key(configs, "server");
    
    for (_index = 0; _index < E_NUM_P(config_data); ++_index)
    {
        /* Loop each config server */
        EXJSON *server = EV_VALUE_P(E_DATA_P(config_data) + _index);
        
        /* Loop each server's config */
        for (_index_j = 0; _index_j < E_NUM_P(server); ++_index_j)
        {
            EXJSON_V *webserver = E_DATA_P(server) + _index_j;
            if ( strncasecmp(EV_NAME_P(webserver), CONFIG_LISTEN) == 0 )
            {
                server_port = *(long *)EV_VALUE_P(webserver);
                _server_fd = socket_create(AF_INET, SOCK_STREAM, 0);
                socket_bind_address(_server_fd, "0.0.0.0", server_port);
                socket_listen(_server_fd, 1000);

#ifdef __linux__
                memset(&ev, 0, sizeof(struct epoll_event));
                ev.data.fd = server_fd;
                ev.events  = EPOLLIN;
                result = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev);
#else
                memset(&ev, 0, sizeof(struct kevent));
                EV_SET(&ev, _server_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
                result = kevent(epoll_fd, &ev, 1, NULL, 0, &ts);
#endif
                assert(result != -1);
            }
        }
    }
    
    /* After adding the file descriptor into listening mode. */
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
                if ( event[epoll_index].filter == EVFILT_READ )
#endif
                {
#ifdef __linux__
                    client_fd = accept(event[epoll_index].data.fd, NULL, NULL);
#else
                    client_fd = accept(event[epoll_index].ident, NULL, NULL);
#endif
                    if ( client_fd )
                    {
                        if ( client_fd == -1 )
                        {
                            perror("Accept: ");
                            continue;
                        }
                        
                        /* if current request get the boundary of 1 million
                         * reset it to zero and restart from zero */
                        if ( dispatch_index == 1000000 ) dispatch_index = 0;
                        
                        /* Dispatch to dispatch_id */
                        dispatch_id = dispatch_index++ % finally_worker_num;
                        
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

/* Get the master process
 * listen the newly coming client and dispatch to the other worker */
void master_process( int server_fd )
{
	int epoll_num, epoll_fd, epoll_index,
		dispatch_id, client_fd, result, dispatch_index = 0;

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
				if ( event[epoll_index].filter == EVFILT_READ )
#endif
				{
					client_fd = accept(server_fd, NULL, NULL);
					if ( client_fd )
					{
					    if ( client_fd == -1 )
                        {
					        perror("Accept: ");
                            continue;
                        }
					    
						/* if current request get the boundary of 1 million
						 * reset it to zero and restart from zero */
						if ( dispatch_index == 1000000 ) dispatch_index = 0;

						/* Dispatch to dispatch_id */
						dispatch_id = dispatch_index++ % finally_worker_num;

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
void http_server_run(int server_fd, void (*callback)(int, EX_REQUEST_T * ))
{
	int _j, epoll_fd, epoll_num, client_fd;

    EX_REQUEST_T request;
    
	size_t  start_pos = 0,  /* parsing http stream start position. */
	        stream_length;  /* the http stream parsing result length, if start_pos equals to stream_length, means the parsing end. */

    /* Client sockaddr */
    struct sockaddr client_addr;
    socklen_t client_addr_len;
	   
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
	struct kevent ev, events[EPOLL_KQUEUE_NUMBER];
	struct timespec ts;
	memset(&ev, 0, sizeof(struct kevent));
	memset(events, 0, sizeof(struct kevent) * EPOLL_KQUEUE_NUMBER);

	ts.tv_sec = ts.tv_nsec = 0;
	EV_SET(&ev, server_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
	kevent(epoll_fd, &ev, 1, NULL, 0, &ts);
#endif

	while ( true )
	{
#ifdef __linux__
		epoll_num = epoll_wait(epoll_fd, events, 100, -1);
#else
		epoll_num = kevent(epoll_fd, NULL, 0, events, EPOLL_KQUEUE_NUMBER, NULL);
#endif
		if ( epoll_num )
		{
			for (_j = 0; _j < epoll_num; ++_j)
			{
                request.keep_alive = 0;

#ifdef __linux__
				if ( events[_j].events & EPOLLRDHUP ) close(events[_j].data.fd);
				if ( events[_j].events & ( EPOLLIN | EPOLLOUT | EPOLLHUP | EPOLLERR ) )
#else
				if ( events[_j].flags == EV_ERROR || events[_j].flags == EV_EOF )
                {
				    close(events[_j].ident);
                    memset(&ev, 0, sizeof(struct kevent));
                    ts.tv_sec = ts.tv_nsec = 0;
                    EV_SET(&ev, events[_j].ident, EVFILT_READ, EV_DELETE, 0, 0, NULL);
                    kevent(epoll_fd, &ev, 1, NULL, 0, &ts);
                    continue;
                }
				if ( events[_j].filter == EVFILT_READ )
#endif
				{
#ifdef __linux__
					if ( events[_j].data.fd == server_fd )
#else
					if ( events[_j].ident == server_fd )
#endif
					{
						client_fd = accept(server_fd, &client_addr, &client_addr_len);
						if ( client_fd )
						{
						    if ( client_fd == -1 )
                            {
						        perror("accept: ");
                                continue;
                            }
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
						    memset(&request, 0, sizeof(EX_REQUEST_T));
						    
							start_pos = 0;
							while ( true )
							{
								EXLIST *stream = parse_http_stream( buff, &start_pos );

								EXLIST_FOREACH(stream, ptr)
								{
									EX_HTTP_HEADER *header = ELV_VALUE_P(ptr);
									
									if ( strncasecmp(HEADER_KEY_P(header), HT_HTTP_BODY ) == 0 )
                                    {
									    request.body = HEADER_VALUE_P(header);
                                    }

									/* Find the host */
									if ( strncasecmp(HEADER_KEY_P(header), HT_HTTP_HOST ) == 0 )
									{
									    request.server_host = HEADER_VALUE_P(header);
									}

									/* Find the request_method */
									if ( strncmp(HEADER_KEY_P(header), HT_REQUEST_METHOD ) == 0 )
									{
									    request.request_method = HEADER_VALUE_P(header);
									}

									/* Find request_uri */
									if ( strncmp(HEADER_KEY_P(header), HT_REQUEST_URL ) == 0 )
									{
									    request.request_url = HEADER_VALUE_P(header);
									}

									/* Find http version */
									if ( strncasecmp(HEADER_KEY_P(header), HT_HTTP_VERSION ) == 0 )
									{
										if ( strncasecmp(HEADER_VALUE_P(header), HT_HTTP_1_0 ) == 0 )
										{
											request.http_version_i = 0;
										}
										else if ( strncasecmp(HEADER_VALUE_P(header), HT_HTTP_1_1 ) == 0 )
										{
                                            request.http_version_i = 1;
											request.keep_alive = 1;
										}
										else
										{
                                            request.http_version_i = 2;
										}
										
										request.http_version = HEADER_VALUE_P(header);
										request.headers = ELV_NEXT_P(ptr);
									}

									/* Find the connection header
									 * if value is keep-alive and http*/
									if ( strncasecmp(HEADER_KEY_P(header), HT_CONNECTION ) == 0 )
									{
										if ( strncasecmp(HEADER_VALUE_P(header), HT_KEEP_ALIVE ) == 0)
										{
											request.keep_alive = 1;
										}
										else
										{
											request.keep_alive = 0;
										}
									}
								} EXLIST_FOREACH_END();

								/* after parsing steps, calling the callback */
								callback(client_fd, &request);

								/* after using, destroy the http stream result. */
								destroy_exlist(stream);
								if ( start_pos == stream_length ) break;
							}
							free(buff);
                            
							/* if not keep alive the connection shutdown other than keep-alive */
							if ( !request.keep_alive )
							{
								shutdown(client_fd, SHUT_WR);
								close(client_fd);
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
