/**
 * File:types.h for project sockets.
 * Author: Josin
 * Email: xeapplee@gmail.com
 * Website: https://www.supjos.cn
 */

#ifndef SOCKETS_TYPES_H
#define SOCKETS_TYPES_H

#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <ctype.h>
#include <netdb.h>

#include <ex_list.h>
#include <exjson.h>

#include <ex_types.h>
#include <ex_socket.h>

enum {
    EX_REQUEST_HTTP_1_0 = 0,
    EX_REQUEST_HTTP_1_1,
    EX_REQUEST_HTTP_2_0
};

typedef struct _EX_REQUEST_T
{
    char     http_version_i;
    char    *server_host;
    char    *request_method;
    char    *request_url;
    char    *http_version;
    char    *content_type;
    void    *headers;
    char    *http_body;
    long     http_body_length;
    int      keep_alive;
} EX_REQUEST_T;

#define EX_HOST     "Host"
#define EX_METHOD   "request_method"
#define EX_URL      "request_uri"
#define EX_VERSION  "http_version"
#define EX_C_TYPE   "Content-Type"
#define EX_KEEP     "keep-alive"
#define EX_LENGTH   "Content-Length"
#define EX_CONNECT  "Connection"
#define EX_HTTP_1_0 "HTTP/1.0"
#define EX_HTTP_1_1 "HTTP/1.1"
#define EX_HTTP_2_0 "HTTP/2.0"
#define EX_BODY     "http_body"

#define EX_CON(all_size, total_size) (&(all_size)), (&(total_size))

EXLIST *ex_parse_http_stream(char *http_request_stream, long stream_length, long *str_len);
EXLIST *ex_parse_query_string(const char *query_string);

void ex_init_request(EX_HTTP_HEADER *, EXLIST_V *, EX_REQUEST_T *);

/* Generate http response string */
char *generate_response_string(int code, char *msg, char *body, int n, ...);
char *generate_request_string(char *method, char *url, char *body, int n, ...);

/* Some apis for special HTTP status code response */
void send_404_response(int _fd, int keep);

#ifdef __linux__
char *parse_proc_cmdline(int pid);
#endif

/* Get the file data */
char *ex_copy_data_from_file(char *file);
char *get_file_data(char *filename);

#endif /* SOCKETS_TYPES_H */
