/**
 * Copyright @2019 Exserver All Rights Reserved.
 */


#ifndef EX_TYPES_H
#define EX_TYPES_H

#include <unistd.h>
#include <fcntl.h>

#ifdef __linux__
    #include <sys/epoll.h>
#else
    #include <sys/time.h>
    #include <sys/event.h>
#endif

typedef void(EVENT_CALLBACK)(long, int);

#define EVENT_SIZE           100
#define EVENT_TIMEOUT        120
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
    char *key;    /* key name */
    char *value;  /* value match key */
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

#ifdef __linux__
    #define EV_READ  EPOLLIN
    #define EV_WRITE EPOLLOUT
#else
    #define EV_READ  EVFILT_READ
    #define EV_WRITE EVFILT_WRITE
#endif

#endif  /* EX_TYPES_H */

