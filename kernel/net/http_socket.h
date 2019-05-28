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
#ifdef __linux__
#include <sys/epoll.h>
#else
#include <sys/event.h>
#include <sys/time.h>
#endif

/* Socket transfer length */
#define SOCKET_FD_LEN CMSG_LEN(sizeof(int))

/* Macro defines epoll & kqueue events number */
#define EPOLL_KQUEUE_NUMBER 50

/* Maxnum of workers */
#define MAX_WORKER_NUMBER 20

/* Some macros for http header parsing */
#define HT_REQUEST_METHOD "request_method", 14
#define HT_REQUEST_URL    "request_uri", 11
#define HT_HTTP_VERSION   "http_version", 12
#define HT_HTTP_1_0       "HTTP/1.0", 8
#define HT_HTTP_1_1       "HTTP/1.1", 8
#define HT_HTTP_2_0       "HTTP/2.0", 8
#define HT_CONNECTION     "Connection", 10
#define HT_KEEP_ALIVE     "keep-alive", 10


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
