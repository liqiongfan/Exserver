/**
 * Copyright @2019 Exserver All rights reserved
 */

#ifndef EX_HTTP_REQUEST_H
#define EX_HTTP_REQUEST_H

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

typedef int   soc_t;
typedef long  sol_t;
typedef void (*FUNC)(int ,EX_REQUEST_T *);

static EXJSON *config    = NULL;
static FUNC    http_back = NULL;
soc_t   process_index, master_number, current_process_number;
pid_t   WORKERS[MAX_WORKERS];
soc_t   WORKER_SOCKETS[MAX_WORKERS][2];

#define GLOBAL_VARS
#define SYSTEM

static GLOBAL_VARS SYSTEM  sol_t  use_send = 0;

#define HT_SERVER         "servers"
#define HT_SYSTEM         "system"
#define HT_SEND_FILE      "send_file"

#define CONFIG_LISTEN     "listen", 6
#define CONFIG_HOST       "host", 4
#define CONFIG_WEBROOT    "webroot", 7
#define CONFIG_INDEX      "index", 5


void ex_gen_worker(int _n, void (*func)(int, EX_REQUEST_T *));
void ex_http_server_start(int fd);
void ex_http_server_from_config();

#endif /* EX_HTTP_REQUEST_H */