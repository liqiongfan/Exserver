/**
 * Copyright @2019 Exserver All Rights Reserved.
 */


#ifndef EX_HTTP_H
#define EX_HTTP_H

#include <fcntl.h>
#include <unistd.h>
#include <ex_log.h>
#include <ex_list.h>
#include <sys/stat.h>
#include <ex_events.h>
#include <ex_config.h>
#include <ex_socket.h>
#include <ex_string.h>
#include <exjson.h>
#include <ex_mine.h>

#ifdef __linux__
#include <sys/sendfile.h>
#else

#include <sys/uio.h>

#endif

#include <ex_http_stream.h>

typedef int  soc_t;
typedef long sol_t;

typedef void (*FUNC)( int, EX_REQUEST_T * );

static EXJSON *config   = NULL, *csystem, *cservers;
static FUNC   http_back = NULL;
soc_t         process_index, master_number, current_process_number;
pid_t         WORKERS[MAX_WORKERS];
soc_t         WORKER_SOCKETS[MAX_WORKERS][2];

#define GLOBAL_VARS
#define SYSTEM

static GLOBAL_VARS SYSTEM  sol_t use_send = 0;

#define HT_SERVER         "servers"
#define HT_SYSTEM         "system"
#define HT_SEND_FILE      "send_file"
#define HT_DAEMON         "daemon"

#define CONFIG_HOST       "host"    , 4
#define CONFIG_INDEX      "index"   , 5
#define CONFIG_LISTEN     "listen"  , 6
#define CONFIG_WEBROOT    "webroot" , 7


void ex_http_loop(int fd, int signo, int efd);

void ex_http_worker(int index, FUNC func);

void ex_http_worker_init(int n, FUNC func);

void ex_http_server(int fd, int signo, int efd);

void ex_http_server_init();
#endif  /* EX_HTTP_H */
