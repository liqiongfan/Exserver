/**
 * Copyright @2019 Exserver All Rights Reserved.
 */

#include <ex_types.h>
#include <exjson.h>
#include <ex_config.h>
#include <ex_http.h>
#include <signal.h>
#include <ex_events.h>

#pragma clang diagnostic push
#pragma ide diagnostic ignored    "missing_default_case"
#pragma ide diagnostic ignored    "UnusedValue"
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma ide diagnostic ignored    "OCUnusedGlobalDeclarationInspection"

static void sigaction_handler(int sig, siginfo_t *si, void *context)
{
    switch (sig)
    {
        case SIGCHLD:
            while(waitpid(-1, NULL, WNOHANG) != -1)
            {
                ;
            }
        
    }
}

void ex_server_parse(int fd, EX_REQUEST_T *req)
{
    int              i,       j,      im,     re,    md;
    char            *wr,     *sh,    *wi,    *qp,    cm[BUFFER_ALLOCATE_SIZE],    *rv,    mr[BUFFER_ALLOCATE_SIZE];
    long             sp,      bl;
    EX_RESPONSE_T   *res;
    
    EXJSON          *server;
    struct stat      fs;
    
    sp =  80;
    bl = im =  0;
    qp = wr = sh =  NULL;
    wi = "index.html";
    
    for (i = 0; i < E_NUM_P(cservers); ++i)
    {
        /* Loop each config server */
        server = EV_VALUE_P(E_DATA_P(cservers) + i);

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

        if ( sh == NULL || wr == NULL )
        {
            return send_500_response(fd, req->keep_alive);
        }
        
        ex_memzero(mr, sizeof(mr));
        
        /* Combine the data into match string */
        if ( sp == 80 ) sprintf(mr, "%s", sh);
        else sprintf(mr, "%s:%ld", sh, sp);

        /* See the matching web root */
        if ( strncasecmp(req->server_host, mr, strlen(mr)) == 0 )
        {
            im = 1; break;
        }
    }
    
    if ( !im ) return send_404_response(fd, req->keep_alive);
    
    ex_memzero(mr, sizeof(mr));
    if ( req->request_url && *(req->request_url) == '/' && *(req->request_url + 1) == '\0')
    {
        rv = wi;
    }
    else
    {
        qp = strchr( req->request_url, '?' );
        if ( qp )
            rv = exsubstr( req->request_url, 1, qp - req->request_url - 1, TRIM_NONE );
        else
            rv = req->request_url + 1;
    }
    sprintf(mr, "%s/%s", wr, rv);
    if ( qp ) ex_memfree(rv);
    
    md = open(mr, O_RDONLY);
    if ( md == -1 )
    {
        if ( errno == EACCES )
        {
            close(md);
            return send_403_response(fd, req->keep_alive);
        }
        close(md);
        return send_404_response(fd, req->keep_alive);
    }
    
    re = fstat(md, &fs);
    if ( re == -1 )
    {
        close(md);
        return send_404_response(fd, req->keep_alive);
    }
    
    sprintf(cm, "Content-Type: %s", ex_get_mine_type(mr));
    rv = ex_copy_data_from_file(mr, &bl);
    ex_memzero(mr, sizeof(mr));
    
#ifdef  __linux__
    sprintf(mr, "Content-Length: %ld",  fs.st_size);
#else
    sprintf(mr, "Content-Length: %lld", fs.st_size);
#endif
    
    if ( req->keep_alive )
    {
        res = genereate_response_t(
            200, "OK", rv, bl, 4, cm, mr, "Connection: keep-alive", "Server: Exserver/1.0"
        );
    }
    else
    {
        res = genereate_response_t(
            200, "OK", rv, bl, 4, cm, mr, "Connection: close", "Server: Exserver/1.0"
        );
    }
    
    write( fd, res->response, sizeof( char ) * res->length );
    ex_memfree(res->response);
    ex_memfree(res);
    ex_memfree(rv);
    close(md);
}

void ex_http_loop(int fd, int signo, int efd)
{
    char           *bf;
    EXLIST         *s;
    EX_REQUEST_T    req;
    EX_HTTP_HEADER *header;
    int             cfd,    rcd;
    long            len,    np;
    
    rcd = WORKER_SOCKETS[process_index][1];
    
    if ( rcd == fd )
    {
        cfd = ex_socket_recv_fd(fd);
        
        ex_make_fd_nonblock(cfd);
        
        ex_add_to_events(efd, cfd);
    }
    else
    {
        len = 0;
        bf  = ex_read_requests2(fd, &len);
        
        if ( bf )
        {
            np = 0;
            
            while (true)
            {
                s = ex_parse_http_stream(bf, len, &np);

                ex_memzero(&req, sizeof(EX_REQUEST_T));

                EXLIST_FOREACH(s, ptr) {
                    header = ELV_VALUE_P(ptr);
                    ex_init_request(header, ptr, &req);
                } EXLIST_FOREACH_END();
                
                if ( http_back ) http_back(fd, &req);
                else ex_server_parse(fd, &req);

                if ( !req.keep_alive )
                {
                    shutdown(fd, SHUT_WR);
                    close(fd); /* After sending the FIN to close the fd. */
                }

                /* Free the exlist HTTP header info. */
                destroy_exlist(s);

                /* When getting the end of the buff */
                if (len == np) break;
            }
        }
        
        /* free the http stream buff */
        ex_memfree(bf);
    }
}

void ex_http_worker(int index, FUNC func)
{
    int     ed,  fd;
    
    http_back       = func;
    process_index   = index;
    
    ed = ex_init_events();
    
    fd = WORKER_SOCKETS[index][1];
    
    if ( fd == 0 ) return ;
    
    ex_add_to_events(ed, fd);
    
    ex_event_loop(ed, ex_http_loop);
}

void ex_http_worker_init(int n, FUNC func)
{
    long    l,      daemon;
    int     i,      re;
    void   *ptr;
    char   *s;
    struct sigaction sig;
    
    l       = 0;
    s       = ex_copy_data_from_file("../conf/exserver.exjson", &l);
    config  = decode_json(s);
    ex_memfree(s);
    
    ex_memzero(&sig, sizeof(sig));
    sig.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &sig, NULL);
    
    sig.sa_sigaction = sigaction_handler;
    sigemptyset(&sig.sa_mask);
    sig.sa_flags = SA_SIGINFO;
    sigaction(SIGCHLD, &sig, NULL);
    
    current_process_number  = n;
    
    ex_memzero(WORKERS, sizeof(WORKERS));
    ex_memzero(WORKER_SOCKETS, sizeof(WORKER_SOCKETS));
    
    cservers = exjson_get_val_from_key(config, HT_SERVER);
    csystem  = exjson_get_val_from_key(config, HT_SYSTEM);
    
    if ( csystem )
    {
        ptr = exjson_get_val_from_key(csystem, HT_SEND_FILE);
        if ( ptr )
        {
            use_send = *(long *)ptr;
        }
        
        ptr   = exjson_get_val_from_key(csystem, HT_DAEMON);
        
        if ( ptr )
        {
            daemon = *(long *)ptr;
            if ( daemon )
            {
                daemon = fork();
                if ( daemon )
                    exit( 0 );
            }
        }
    }
    
    for ( i = 0; i < n; ++i )
    {
        re = socketpair(AF_UNIX, SOCK_STREAM, 0, WORKER_SOCKETS[i]);
        if ( re == -1 )
        {
            ex_logger(LOG_ERROR, "socketpair failed; " EX_ERROR_MSG_FMT, EX_ERROR_MSG_STR);
            return ;
        }
        
        re = fork();
        if ( re == -1 )
        {
            ex_logger(LOG_ERROR, "socketpair failed; " EX_ERROR_MSG_FMT, EX_ERROR_MSG_STR);
            return ;
        }
        if ( re == 0 )
        {
            ex_http_worker(i, func);
            exit(0);
        }
        else
        {
            WORKERS[i] = re;
        }
    }
}

void ex_http_server(int fd, int signo, int efd)
{
    int     cfd;
    int     did;
    
    if ( master_number == 1000000 ) master_number = 0;
    
again:
    cfd = accept(fd, NULL, NULL);
    
    if ( cfd == -1 )
    {
        if ( errno == EAGAIN || errno == EWOULDBLOCK )
        {
            goto again;
        }
        else
        {
            ex_logger(LOG_INFO, "System wrong; " EX_ERROR_MSG_FMT, EX_ERROR_MSG_STR);
            return ;
        }
    }
    
    did = master_number++ % current_process_number;
    
    ex_socket_send_fd(WORKER_SOCKETS[did][0], cfd);
    
    close(cfd);
}

void ex_http_server_init()
{
    long      sp;
    int       i,    j,   fd,   efd;
    EXJSON   *server;
    EXJSON_V *webserver;
    
    master_number = 0;
    
    efd = ex_init_events();
    
    for (i = 0; i < E_NUM_P(cservers); ++i)
    {
        /* Loop each config server */
        server = EV_VALUE_P(E_DATA_P(cservers) + i);
        
        /* Loop each server's config */
        for (j = 0; j < E_NUM_P(server); ++j)
        {
            webserver = E_DATA_P(server) + j;
            
            if ( strncasecmp(EV_NAME_P(webserver), CONFIG_LISTEN) == 0 )
            {
                sp = *(long *)EV_VALUE_P(webserver);
                
                fd = ex_create_socket(NULL, (uint16_t)sp);
                ex_listen_socket(fd, 10000);
                
                /* add to event listen queue */
                ex_add_to_events(efd, fd);
            }
        }
    }
    
    /* after adding the data to loop from the events */
    ex_event_loop(efd, ex_http_server);
}


#pragma clang diagnostic pop