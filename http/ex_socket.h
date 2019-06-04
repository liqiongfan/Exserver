/**
 * Copyright @2019 Exserver All rights reserved
 */


#ifndef EX_SOCKET_H
#define EX_SOCKET_H

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SOCKET_FD_LEN CMSG_LEN(sizeof(int))

/* if your machine is much faster, incr this buff size */
#define BUFFER_SIZE 512

#define ex_server_accept(fd, addr, len) accept(fd, addr, len)

int ex_make_fd_nonblock(int fd);
int ex_create_socket(const char *host, uint16_t port);
int ex_listen_socket(int fd, int bl);
int ex_connect_socket(int fd, const char *host, uint16_t port);

int ex_socket_send_fd(int fd, int fd_to_send);
int ex_socket_recv_fd(int fd);

char *ex_read_num_data(int _fd, long len);
char *ex_read_requests(int _fd, long *len);

#endif /* EX_SOCKET_H */
