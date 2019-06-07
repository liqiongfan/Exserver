/**
 * Copyright @2019 Exserver All rights reserved
 */

#include <errno.h>

#include <ex_log.h>
#include <ex_string.h>
#include <ex_config.h>
#include <ex_socket.h>
#include <ex_http_stream.h>

int ex_create_socket(const char *host, uint16_t port)
{
	int sid, d, k;

	struct sockaddr_in addr;

	sid = socket(AF_INET, SOCK_STREAM, 0);
	if ( sid == -1 )
	{
		ex_logger(LOG_ERROR, "Create socket failed; " EX_ERROR_MSG_FMT, EX_ERROR_MSG_STR);
	}
	assert( sid != -1 );

	k = 1;
	setsockopt(sid, SOL_SOCKET, SO_KEEPALIVE, &k, sizeof( int ));
	setsockopt(sid, SOL_SOCKET, SO_REUSEADDR, &k, sizeof( int ));
	setsockopt(sid, SOL_SOCKET, SO_REUSEPORT, &k, sizeof( int ));

	/* if host is NULL */
	if ( host )
	{
		inet_pton(AF_INET, host, &addr.sin_addr);
	}
	else
	{
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	addr.sin_port = htons(port);
	addr.sin_family = AF_INET;

	d = bind(sid, (struct sockaddr *)&addr, sizeof(addr));
	if ( d == -1 )
	{
		ex_logger(LOG_ERROR, "Bind to socket: %d failed; " EX_ERROR_MSG_FMT, sid, EX_ERROR_MSG_STR);
	}
	assert(d != -1);
	
	return sid;
}

int ex_listen_socket(int fd, int bl)
{
    int r;
    if ( !fd )
    {
        ex_logger(LOG_ERROR, "Listen socket: %d wrong; fd is:%d\n", fd);
        assert( fd );
    }
    
    r = listen(fd, bl);
    if ( r == -1 )
    {
        ex_logger(LOG_ERROR, "Listen socket: %d faild; " EX_ERROR_MSG_FMT, fd, EX_ERROR_MSG_STR);
        assert( r != -1 );
    }
    return 1;
}

int ex_connect_socket(int _server_fd, const char *host, uint16_t port)
{
	int _result;
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));

	addr.sin_port = htons(port);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(host);

	_result = connect(_server_fd, (struct sockaddr *)&addr, sizeof(addr));
	if ( _result )
	{
		ex_logger(LOG_ERROR, "Connect: %d error; " EX_ERROR_MSG_FMT, _server_fd, EX_ERROR_MSG_STR);
	}
	return _result;
}

int ex_make_fd_nonblock(int fd)
{
	int result, flags = fcntl(fd, F_GETFL);
	result = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
	if ( result == -1 )
	{
		ex_logger(LOG_ERROR, "Set fd: %d nonblocking failed; " EX_ERROR_MSG_FMT, fd, EX_ERROR_MSG_STR);
	}
	return result;
}

char *ex_read_num_data(int _fd, long len)
{
    ssize_t rn;
    char    *r, tf[BUFFER_SIZE];
    long    en, un, ln;
    
    r = malloc(sizeof(char) * ( len ) );
    if ( r == NULL ) return NULL;
    ex_memzero(r, sizeof(char) * len);
    
    un = 0;
    
    ln = len;
    
    for ( ;; )
    {
        if ( ln == 0 ) break;
        
        if ( ln >= BUFFER_SIZE )
            en = BUFFER_SIZE;
        else
            en = ln;
        
        ex_memzero(tf, sizeof(tf));
        rn = read(_fd, tf, sizeof(char) * en);
        if ( rn == -1
            && (errno == EAGAIN || errno == EWOULDBLOCK) )
        {
            continue;
        }
        if ( rn > 0 )
        {
            ex_copymem( r + un, tf, sizeof(char) * en );
            un += rn;
            ln -= rn;
        }
    }
    
    return r;
}

long ex_check_http_stream(const char *stream, long stream_len)
{
    
    int     i,  rm;
    long    bl, nl, ll;
    
    rm  = i = 0;
    
    if ( ex_str3cmp(stream, "GET") )
    {
        i = 2;
        rm = HTTP_GET;
    }
    else if ( ex_str3cmp(stream, "PUT") )
    {
        i = 2;
        rm = HTTP_PUT;
    }
    else if ( ex_str4cmp(stream, "POST") )
    {
        i = 3;
        rm = HTTP_POST;
    }
    else if ( ex_str7cmp(stream, "OPTIONS") )
    {
        i = 6;
        rm = HTTP_OPTIONS;
    }
    else if ( ex_str6cmp(stream, "DELETE") )
    {
        i = 5;
        rm = HTTP_DELETE;
    }
    
    for ( ; i < stream_len; i++ )
    {
        if ( ex_str13ncmp(stream +i, "content-length") )
        {
            bl = ex_get_cl(stream, i);
    
            if ( bl == -1 ) return -1;
            
            if ( bl == 0 ) break;
    
            for ( nl = i; nl < stream_len ; nl++ )
            {
                if ( ex_str4cmp( stream + nl, "\r\n\r\n") )
                {
                    ll = stream_len - nl - 4;
            
                    ll = bl - ll;
                    if ( ll == 0 )
                    {
                        return 0;
                    }
                    else
                    {
                        return ll;
                    }
                }
            }
        }
    }
    
    /* Not need Content-Length */
    if ( stream[stream_len - 1] == '\n' &&
         stream[stream_len - 2] == '\r' &&
         stream[stream_len - 3] == '\n' &&
         stream[stream_len - 4] == '\r' )
        return 0;
    else
        return -1;
}

char *ex_read_requests2(int _fd, long *len)
{
	char *r,  *v,  *nv,  tf[BUFFER_SIZE];
	long  rn,  an,  un,  ll;

	un = 0;
	an = 0;
	r  = NULL;

	for ( ;; )
	{
		ex_memzero(tf, sizeof(tf));
		rn = read(_fd, tf, sizeof(char) * BUFFER_SIZE);
		if ( rn == -1 && errno == EAGAIN ) continue;
		if ( rn == -1 && errno == EPIPE ) return NULL;
		if ( rn == 0 ) { close(_fd); return r; }
		
		an  += rn;
	   *len += rn;
		v = realloc(r, sizeof(char) * an);
		if ( v == NULL )
		{
			ex_memfree(r);
			return NULL;
		}
		r = v;

		ex_copymem(r + un, tf, sizeof(char) * rn);
		un += rn;
		
		ll = ex_check_http_stream(r, an);
		if ( ll == -1 ) continue;
		
		if ( ll == 0 ) return r;
		else
        {
            nv = ex_read_num_data( _fd, ll );
    
            ex_logger(LOG_ERROR, "[[%s:%d]]", nv, ll);
            
            v  = realloc( r, sizeof( char ) * ( an + ll ) );
            if ( v == NULL )
            {
                ex_memfree( r );
                return NULL;
            }
            r = v;
           *len += ll;
            ex_copymem( r + un, nv, sizeof( char ) * ll );
            ex_memfree( nv );
            return r;
        }
	}
}

int ex_socket_send_fd(int fd, int fd_to_send)
{
    char buff[2];
    memset(buff, 0, sizeof(buff));
    
    struct iovec io_vec[1];
    memset(io_vec, 0, sizeof(struct iovec));
    io_vec->iov_len = 2;
    io_vec->iov_base = buff;
    
    struct msghdr msg_hdr;
    memset(&msg_hdr, 0, sizeof(struct msghdr));
    msg_hdr.msg_iov     = io_vec;
    msg_hdr.msg_iovlen  = 1;
    msg_hdr.msg_name    = NULL;
    msg_hdr.msg_namelen = 0;
    
    struct cmsghdr cmptr;
    memset(&cmptr, 0, sizeof(struct cmsghdr));
    
    if ( fd_to_send < 0 )
    {
        msg_hdr.msg_control    = NULL;
        msg_hdr.msg_controllen = 0;
        buff[1] = ~fd_to_send;
        if ( buff[1] == 0 )
            buff[1] = 1; /* nonzero means failure */
    }
    else
    {
        cmptr.cmsg_level = SOL_SOCKET;
        cmptr.cmsg_type  = SCM_RIGHTS;
        cmptr.cmsg_len   = SOCKET_FD_LEN;
        
        msg_hdr.msg_control = &cmptr;
        msg_hdr.msg_controllen = SOCKET_FD_LEN;
        *(int *)CMSG_DATA(&cmptr) = fd_to_send;
        buff[1] = 0; /* zero means ok */
    }
    
    /* first char is 0 */
    buff[0] = 0;
    
    if ( sendmsg(fd, &msg_hdr, 0) != 2 )
        return (-1);
    
    return (0);
}

int ex_socket_recv_fd(int fd)
{
    char  buff[256];
    struct cmsghdr cmptr;
    memset(&cmptr, 0, sizeof(struct cmsghdr));
    struct iovec io_vec[1];
    memset(io_vec, 0, sizeof(struct iovec));
    struct msghdr  msg_hdr;
    memset(&msg_hdr, 0, sizeof(struct msghdr));
    
    io_vec[0].iov_base  = buff;
    io_vec[0].iov_len   = sizeof(buff);
    msg_hdr.msg_iov     = io_vec;
    msg_hdr.msg_iovlen  = 1;
    msg_hdr.msg_name    = NULL;
    msg_hdr.msg_namelen = 0;
    
    msg_hdr.msg_control    = &cmptr;
    msg_hdr.msg_controllen = SOCKET_FD_LEN;
    
    recvmsg(fd, &msg_hdr, 0);
    
    return *(int *)CMSG_DATA(&cmptr);
}


























