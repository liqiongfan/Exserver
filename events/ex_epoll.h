/**
 * Copyright @2019 Exserver All Rights Reserved.
 */


#ifndef EXSERVER_EX_EPOLL_H
#define EXSERVER_EX_EPOLL_H

#include <ex_string.h>
#ifdef __linux__
#include <sys/epoll.h>

int ex_init_events();
int ex_add_to_events(int fd, int sd);
int ex_del_from_events(int fd, int sd);
void ex_event_loop(int fd, void (*func)(int, int, int));

#endif
#endif /* EXSERVER_EX_EPOLL_H */
