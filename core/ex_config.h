/**
 * Copyright @2019 Exserver All rights reserved
 */

#ifndef EX_CONFIG_H
#define EX_CONFIG_H

#ifdef __cplusplus
	#ifndef __BEGIN_DECLS
		#define __BEGIN_DECLS extern "C" {
	#endif
	#ifndef __END_DECLS
		#define __END_DECLS };
	#endif
#else
	#ifndef __BEGIN_DECLS
		#define __BEGIN_DECLS
	#endif
	#ifndef __END_DECLS
		#define __END_DECLS
	#endif
#endif

#ifndef TRUE
	#define TRUE 1
#endif
#ifndef FALSE
	#define FALSE 0
#endif

#define MAX_WORKERS 256

enum {
	LOG_ERROR = 1,
	LOG_INFO
};

/* Turn on the debug mode */
#define EX_DEBUG FALSE
#define EX_LOG_ERROR_FILE_NAME "ex_log.log"
#define EX_LOG_INFO_FILE_NAME  "ex_log_info.log"


#endif /* EX_CONFIG_H */