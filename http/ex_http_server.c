/**
 * Copyright @2019 Exserver All rights reserved
 */

#include <ex_http_server.h>

/* Parse the http web browser */
void ex_parser_https(int fd, EX_REQUEST_T *req)
{
	int     i,    j,  im, ffd;
	long    sp,   sf, len;
	EXJSON *cd,  *cs;
	char    *wr, *sh, *qp, *wi, *by, *re, *rru, *mime, cm[BUFFER_ALLOCATE_SIZE], mr[BUFFER_ALLOCATE_SIZE];

	sp  = 80;
	len = sf = im = 0;
	by  = qp = wr = sh = NULL;
	wi  = "index.html";
	ex_memzero(mr, sizeof(mr));

	cd = exjson_get_val_from_key(config, HT_SERVER);
	cs = exjson_get_val_from_key(config, HT_SYSTEM);
	if ( cs )
	{
		/* Turn on the senfile or not */
		sf = *(long *)exjson_get_val_from_key(cs, HT_SEND_FILE);
	}

	for (i = 0; i < E_NUM_P(cd); ++i)
	{
		/* Loop each config server */
		EXJSON *server = EV_VALUE_P(E_DATA_P(cd) + i);

		/* Loop each server's config */
		for (j = 0; j < E_NUM_P(server); ++j)
		{
			EXJSON_V *webserver = E_DATA_P(server) + j;
			if ( strncasecmp(EV_NAME_P(webserver), CONFIG_WEBROOT) == 0 )
			{
				wr = (char *)EV_VALUE_P(webserver);
			}
			if ( strncasecmp(EV_NAME_P(webserver), CONFIG_HOST) == 0 )
			{
				sh = (char *)EV_VALUE_P(webserver);
			}
			if ( strncasecmp(EV_NAME_P(webserver), CONFIG_LISTEN) == 0 )
			{
				sp = *(long *)EV_VALUE_P(webserver);
			}
			if ( strncasecmp(EV_NAME_P(webserver), CONFIG_INDEX) == 0 )
			{
				wi = (char *)EV_VALUE_P(webserver);
			}
		}

		/* Combine the data into match string */
		if ( sp == 80 ) sprintf(mr, "%s", sh);
		else sprintf(mr, "%s:%ld", sh, sp);

		/* See the matching web root */
		if ( strncasecmp(req->server_host, mr, strlen(mr)) == 0 )
		{
			im = 1; break;
		}
	}

	/* Get whether host is matched or not
	 * if not matched send 404 not found to client. */
	if ( !im ) {
		return send_404_response(fd, req->keep_alive);
	}

	/* Now real_request_url has the form like: /index.html */
	if ( req->request_url && *(req->request_url) == '/' && *(req->request_url + 1) == '\0')
	{
		rru = wi;
	}
	else
	{
		qp = strchr( req->request_url, '?' );
		if ( qp )
			rru = exsubstr( req->request_url, 1, qp - req->request_url, TRIM_NONE );
		else
			rru = req->request_url + 1;
	}

	/* Find the file data */
	ex_memzero(mr, sizeof(mr));
	sprintf(mr, "%s/%s", wr, rru);
	if ( qp )
	{
		ex_memfree(rru);
	}

	/* Open the file */
	ffd = open(mr, O_RDONLY);
	if ( ffd == -1 ) {
		return send_404_response(fd, req->keep_alive);
	}

	ex_memzero(cm, sizeof(cm));
	mime = ex_get_mine_type(mr);
	sprintf(cm, "Content-Type: %s", mime);

	/* Get the file stat info. */
	struct stat file_stat;
	fstat(ffd, &file_stat);
#ifdef __linux__
	if ( !sf ) {
		by = ex_copy_data_from_file(mr, &len);
	}
#endif
	/* Set the Content-Length according the file data */
	ex_memzero(mr, sizeof(mr));
#ifdef __linux__
	sprintf(mr, "Content-Length: %ld", file_stat.st_size);
#else
	sprintf(mr, "Content-Length: %lld", file_stat.st_size);
#endif
	if ( req->keep_alive )
	{
		re = generate_response_string(200, "OK", by, 3, mr, cm, "Connection: keep-alive");
	}
	else
	{
		re = generate_response_string(200, "OK", by, 3, mr, cm, "Connection: close");
	}

#ifdef __linux__
	write(fd, re, strlen(re));
	if ( sf )
	{
		sendfile(fd, ffd, NULL, (size_t)file_stat.st_size);
	}
#else
	struct iovec io_vec;
    io_vec.iov_base = re;
    io_vec.iov_len = strlen(re);
    struct sf_hdtr sf_hdtr1;
    sf_hdtr1.headers = &io_vec;
    sf_hdtr1.hdr_cnt = 1;
    sendfile( ffd, fd, 0, ( off_t * )file_stat.st_size, &sf_hdtr1, 0 );
#endif
	close(ffd);
	ex_memfree(re);
	if ( by ) ex_memfree(by);
}

static void ex_http_worker_run(int fd, int signo, int eid)
{
	/* fd is the descriptor which need to be read from,
	 * signo is the signal number, and eid is the event fd */
	EXLIST      *s;
	EXLIST_V    *ptr;
	EX_REQUEST_T req;
	char        *buff,  *re;
	long         bl,    np;
	int          cfd,   cld;

	/* Initialise the default value to zero */
	np = bl = 0;

	/* Get the read fd with each WORKER socket */
	cfd = WORKER_SOCKETS[process_index][1];

	if ( cfd == fd )
	{
		cld = ex_socket_recv_fd(cfd);

		ex_make_fd_nonblock(cld);

		ex_add_to_events(eid, cld);
	}
	else
	{
		buff = ex_read_requests(fd, &bl);

		if ( !buff )
		{
			ex_del_from_events(eid, fd);

			close(fd);

			return ;
		}

		while ( true )
		{
			s = ex_parse_http_stream( buff, bl, &np );

			ex_memzero(&req, sizeof(EX_REQUEST_T));

			EXLIST_FOREACH(s, ptr) {
				EX_HTTP_HEADER *header = ELV_VALUE_P(ptr);
				ex_init_request(header, ptr, &req);
			} EXLIST_FOREACH_END();

			/* Call the back function */
			ex_parser_https(fd, &req);

			if ( !req.keep_alive )
			{
				shutdown(fd, SHUT_WR);
			}

			destroy_exlist(s);
			ex_memfree(buff);

			if ( bl == np ) break;
		}
	}
}

static void ex_http_worker(int index, void (*HTTP_FUNC)(int ,EX_REQUEST_T *))
{
	int fd, sid;

	/* Receive newly dispatched client from master process */
	sid = WORKER_SOCKETS[index][1];

	http_back = HTTP_FUNC;

	process_index = index;

	fd = ex_init_events();

	ex_add_to_events(fd, sid);

	ex_event_loop(fd, ex_http_worker_run);
}

void ex_gen_worker(int _n, void (*func)(int, EX_REQUEST_T *))
{
	char    *cd;
	int      i, pid, re;

	current_process_number = _n;
	if ( _n > MAX_WORKERS )
	{
		ex_logger(LOG_ERROR, "Max worker is: %d", MAX_WORKERS);
		return ;
	}
	assert(_n <= MAX_WORKERS);

	/* Parsing the http configs */
	cd     = get_file_data("../conf/exserver.exjson");
	config = decode_json(cd);
	free(cd);

	for ( i = 0; i < _n; ++i ) {

		re = socketpair(AF_UNIX, SOCK_STREAM, 0, WORKER_SOCKETS[i]);
		if ( re == -1 )
		{
			ex_logger(LOG_ERROR, "Create socket unix domain faild; " EX_ERROR_MSG_FMT, EX_ERROR_MSG_STR );
			continue ;
		}

		pid = fork();
		if ( pid == -1 )
		{
			ex_logger(LOG_ERROR, "Create process worker faild; " EX_ERROR_MSG_FMT, EX_ERROR_MSG_STR );
			continue ;
		}
		if ( pid == 0 )
		{
			ex_http_worker(i, func);
			exit(0); /* Child process terminate when func returned. */
		}
		else { WORKERS[i] = pid; }
	}
}

void ex_htp_server_master_process(int fd, int signo, int efd)
{
	int cid, did;

	cid = accept(fd, NULL, NULL);
	if ( cid == -1 )
	{
		ex_logger(LOG_ERROR, "Accept error; " EX_ERROR_MSG_FMT, EX_ERROR_MSG_STR);
		return ;
	}

	did = master_number % current_process_number;

	ex_socket_send_fd(WORKER_SOCKETS[did][0], cid);
	close(cid);

	master_number++;
}

void ex_http_server_start(int fd)
{
	int sfd;
	master_number = 0;

	sfd = ex_init_events();
	ex_add_to_events(sfd, fd);
	ex_event_loop(sfd, ex_htp_server_master_process);
}

void ex_http_server_from_config()
{
	long    sp;
	EXJSON *cd;
	int     i,  j, fd, event_fd;

	event_fd = ex_init_events();

	cd = exjson_get_val_from_key(config, HT_SERVER);

	for (i = 0; i < E_NUM_P(cd); ++i)
	{
		/* Loop each config server */
		EXJSON *server = EV_VALUE_P(E_DATA_P(cd) + i);

		/* Loop each server's config */
		for (j = 0; j < E_NUM_P(server); ++j)
		{
			EXJSON_V *webserver = E_DATA_P(server) + j;

			if ( strncasecmp(EV_NAME_P(webserver), CONFIG_LISTEN) == 0 )
			{
				sp = *(long *)EV_VALUE_P(webserver);

				fd = ex_create_socket("0.0.0.0", (uint16_t)sp);
				ex_listen_socket(fd, 1000);

				/* add to event listen queue */
				ex_add_to_events(event_fd, fd);
			}
		}
	}

	/* after adding the data to loop from the events */
	ex_event_loop(event_fd, ex_htp_server_master_process);
}