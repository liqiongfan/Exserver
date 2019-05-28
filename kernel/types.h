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
#include "exlist.h"

#define BUFFER_SIZE 512
#define BUFFER_ALLOCATE_SIZE 256
#ifndef true
    #define true 1
#endif
#ifndef false
    #define false 0
#endif
#ifdef __cplusplus
    #define EMPTY_PTR nullptr
#else
    #define EMPTY_PTR NULL
#endif

typedef struct _HTTP_HEADER
{
    char *key;    /* HTTP key name */
    char *value;  /* HTTP value match key */
} EX_HTTP_HEADER;

#define EX_EQUAL_DATA EX_HTTP_HEADER

#define HEADER_KEY(header) (header).key
#define HEADER_KEY_P(header) HEADER_KEY(*(header))
#define HEADER_VALUE(header) (header).value
#define HEADER_VALUE_P(header) HEADER_VALUE(*(header))
EX_HTTP_HEADER *INIT_HEADER();

enum HTTP_METHOD_KIND { GET = 1, PUT, POST, DELETE, OPTIONS, HEAD };
enum HTTP_KEYWORDS{ HTTP_METHOD = 0, HTTP_URI, HTTP_VERSION };
enum TRIM_MODE{ TRIM_NONE = 0, TRIM_LEFT, TRIM_RIGHT, TRIM_BOTH };
/* Trim the substring */
char *exsubstr(const char *source, size_t start, size_t length, int mode);
/* Trim the source string */
char *extrim(const char *source, int mode);

/* Get the http stream from socket */
char *get_socket_stream_data(int __sock_fd, size_t *stream_length);
/* Parse the http stream */
EXLIST *parse_http_stream(const char *http_request_stream, size_t *str_len);
/* Parse the query string or form data */
EXLIST *parse_query_string(const char *query_string);
/* Generate http response string */
char *generate_response_string(int code, char *msg, char *body, int n, ...);
char *generate_request_string(char *method, char *url, char *body, int n, ...);
#define EX_CON(all_size, total_size) &(all_size), &(total_size)
void _ex_strncat_(char **dest, const char *source, size_t *origin_size, size_t *used_num);
#endif /* SOCKETS_TYPES_H */
