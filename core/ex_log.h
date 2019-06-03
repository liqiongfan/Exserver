/**
 * Copyright @2019 Exserver All rights reserved
 */

#ifndef EX_LOG_H
#define EX_LOG_H

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <ex_config.h>

#if EX_DEBUG
#define ex_printf(fmt, ...) \
{\
	printf(fmt, __VA_ARGS__);\
}
#else
#define ex_printf(fmt, ...)
#endif

#define EX_ERROR_MSG_FMT "error code: %d; error message: %s"
#define EX_ERROR_MSG_STR  errno, strerror(errno)

void ex_logger(int level, char *info, ...);


#endif /* EX_LOG_H */

