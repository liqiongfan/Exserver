/**
 * File: http_socket.h for project sockets
 * Email: xeapplee@gmail.com
 * website: https://www.supjos.cn
 * Author: Josin 
 * Copyright (c) Exserver-2019 all rights reserved
 */

#ifndef SOCKETS_HTTP_SOCKET_H
#define SOCKETS_HTTP_SOCKET_H

#include "../exlist.h"
#include <sys/epoll.h>

#define SOCKET_FD_LEN CMSG_LEN(sizeof(int))

/* Maxnum of workers */
#define MAX_WORKER_NUMBER 20

/* The data for data store */
static int pids[MAX_WORKER_NUMBER];
static int fd_sockets[MAX_WORKER_NUMBER][2];
static int finally_worker_num = 0;

/* Worker process
 * Unix socket for file descriptor send and recv */
int socket_recv_fd(int fd);
int socket_send_fd(int fd, int fd_to_send);
void generate_worker(
		int worker_num,
		void (*url_callback)(int fd, EXLIST *header, char *method, char *url, int keep_alive));
void master_process( int server_fd );

/* Init the HTTP server based on TCP */
int http_server_init(const char *host, uint32_t port, int backlog);
/* Run the http server */
void http_server_run(int server_fd, void (*callback)(int fd, EXLIST *header, char *method, char *url, int keep_alive));


#endif /* SOCKETS_HTTP_SOCKET_H */
