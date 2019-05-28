/**
 * File:socket.h for project sockets.
 * Author: Josin
 * Email: xeapplee@gmail.com
 * Website: https://www.supjos.cn
 */

#ifndef SOCKETS_SOCKET_H
#define SOCKETS_SOCKET_H

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


typedef struct sockaddr_in server_addr;

int socket_create(int domain, int type, int protocol);
void socket_bind_address(int _server_fd, const char *host, uint32_t port);
void socket_listen(int _server_fd, int backlog);
void socket_connect(int _server_fd, const char *host, uint32_t port);
void socket_nonblock(int _socket_fd);

#endif /* SOCKETS_SOCKET_H */
