/**
 * File:socket.c for project sockets.
 * Author: Josin
 * Email: xeapplee@gmail.com
 * Website: https://www.supjos.cn
 */

#include "socket.h"

/* Get the socket file descriptor,
 * if error occurred process will exit
 * return: server socket fd. */
int socket_create(int domain, int type, int protocol)
{
    int keepalive = 1, reuse = 1;
    int _server_fd = socket(domain, type, protocol);
    assert( _server_fd != -1 );
    
    setsockopt(_server_fd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof( int ));
    setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof( int ));
    setsockopt(_server_fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof( int ));
    
    return _server_fd;
}

/* Bind the socket with the given host & port
 * if error assertion fail otherwise nothing happen */
void socket_bind_address(int _server_fd, const char *host, uint32_t port)
{
    int _result;
    server_addr addr;
    memset(&addr, 0, sizeof(server_addr));
    
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;
    // addr.sin_len = sizeof(server_addr);
    addr.sin_addr.s_addr = inet_addr(host);
    
    _result = bind(_server_fd, (struct sockaddr *)&addr, sizeof(server_addr));
    assert(_result != -1);
}

/* Listen socket with the backlog
 * if backlog is zero, 100 was set others backlog */
void socket_listen(int _server_fd, int backlog)
{
	int result, result_backlog = !backlog ? 100 : backlog;

	result_backlog = listen(_server_fd, result_backlog);
	assert(result_backlog != -1);
}

/* Connect to the given host & port
 * if success nothing return otherwise assertion failed */
void socket_connect(int _server_fd, const char *host, uint32_t port)
{
    int _result;
    server_addr addr;
    memset(&addr, 0, sizeof(server_addr));
    
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;
    // addr.sin_len = sizeof(server_addr);
    addr.sin_addr.s_addr = inet_addr(host);
    
    _result = connect(_server_fd, (struct sockaddr *)&addr, sizeof(server_addr));
    assert(_result != -1);
}

/* Make the given socket nonblocking */
void socket_nonblock(int _socket_fd)
{
    int result, flags = fcntl(_socket_fd, F_GETFL);
    result = fcntl(_socket_fd, F_SETFL, flags | O_NONBLOCK);
    assert(result != -1);
}







