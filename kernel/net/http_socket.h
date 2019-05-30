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
#include "../exjson/exjson.h"
#include <sys/stat.h>

#ifdef __linux__
#include <sys/sendfile.h>
#include <sys/epoll.h>
#else
#include <sys/event.h>
#include <sys/time.h>
#include <sys/uio.h>
#endif

/* Socket transfer length */
#define SOCKET_FD_LEN CMSG_LEN(sizeof(int))

/* Macro defines epoll & kqueue events number */
#define EPOLL_KQUEUE_NUMBER 100

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
#define HT_HTTP_HOST      "Host", 4
#define HT_HTTP_BODY      "http_body", 9

#define CONFIG_LISTEN     "listen", 6
#define CONFIG_HOST       "host", 4
#define CONFIG_WEBROOT    "webroot", 7
#define CONFIG_INDEX      "index", 5


/* The data for data store */
static int pids[MAX_WORKER_NUMBER];
static int fd_sockets[MAX_WORKER_NUMBER][2];
static int finally_worker_num = 0;

/* Exserver config */
EXJSON *configs;

typedef struct _EX_REQUEST_T
{
    unsigned char    http_version_i; /* 0: HTTP/1.0 1: HTTP/1.1 2: HTTP/2.0 */
    unsigned char    keep_alive;     /* Is keep-alive connection */
    char            *request_method; /* Request method, like: GET | POST | PUT */
    char            *request_url;    /* Request URL */
    char            *server_host;    /* Host */
    char            *http_version;   /* HTTP version string such as HTTP/1.1 */
    EXLIST_V        *headers;        /* HTTP headers except headers before */
    char            *body;           /* HTTP body */
} EX_REQUEST_T;

typedef struct _EX_RESPONSE_T
{
    unsigned char    http_version_i; /* 0: HTTP/1.0 1: HTTP/1.1 2: HTTP/2.0 */
    unsigned char    keep_alive;     /* Is keep-alive connection */
    char            *http_version;   /* HTTP version string */
    char            *status_msg;     /* HTTP status message */
    char            *headers;        /* HTTP response headers */
    char            *body;           /* HTTP response body */
    int              status_code;    /* HTTP status code */
} EX_RESPONSE_T;

/* Worker process
 * Unix socket for file descriptor send and recv */
int socket_recv_fd(int fd);
int socket_send_fd(int fd, int fd_to_send);
void generate_worker( int worker_num, void (*url_callback)(int, EX_REQUEST_T *));
void master_process( int server_fd );
void static_server_start();

/* Init the HTTP server based on TCP */
int http_server_init(const char *host, uint32_t port, int backlog);
/* Run the http server */
void http_server_run(int server_fd, void (*callback)(int, EX_REQUEST_T *));


#endif /* SOCKETS_HTTP_SOCKET_H */
