/**
 * Copyright @2019 Exserver All Rights Reserved.
 */

#include <ex_log.h>
#include <ex_epoll.h>
#include <ex_types.h>

#ifdef __linux__

inline int ex_init_events()
{
    int fd;
    
    fd = epoll_create(1);
    if ( fd == -1 )
    {
        ex_logger(LOG_ERROR, "Create epoll fd failed; " EX_ERROR_MSG_FMT, EX_ERROR_MSG_STR);
        return -1;
    }
    return fd;
}

inline int ex_add_to_events(int fd, int sd)
{
    int re;
    struct epoll_event ev;
    ex_memzero(&ev, sizeof(ev));
    
    ev.data.fd = sd;
    ev.events = EV_READ;
    
    re = epoll_ctl(fd, EPOLL_CTL_ADD, sd, &ev);
    if ( re == -1 )
    {
        ex_logger(LOG_ERROR, "Add fd: %d to events faild; " EX_ERROR_MSG_FMT, sd, EX_ERROR_MSG_STR );
        return -1;
    }
    return 1;
}

inline int ex_del_from_events(int fd, int sd)
{
    int re;
    
    re = epoll_ctl(fd, EPOLL_CTL_DEL, sd, NULL);
    if ( re == -1 )
    {
        ex_logger(LOG_ERROR, "Delete fd: %d from events faild; " EX_ERROR_MSG_FMT, sd, EX_ERROR_MSG_STR );
        return -1;
    }
    return 1;
}

inline void ex_event_loop(int fd, void (*func)(int, int, int))
{
    int                 en, i;
    struct epoll_event  event[EVENT_SIZE];
    
    while ( true )
    {
        ex_memzero(event, sizeof(event));
        en = epoll_wait(fd, event, EVENT_SIZE, EVENT_TIMEOUT * 1000);
        
        if ( en == -1 )
        {
            ex_logger(LOG_ERROR, "Epoll waiting error; " EX_ERROR_MSG_FMT, EX_ERROR_MSG_STR );
            continue;
        }
        
        if ( en )
        {
            for ( i = 0; i < en; i++ )
            {
                if ( event[i].events & ( EPOLLRDHUP ) )
                {
                    close(event[i].data.fd);
                }
                else if ( event[i].events & ( EPOLLIN | EPOLLOUT | EPOLLHUP | EPOLLERR ) )
                {
                    func(event[i].data.fd, event[i].events, fd);
                }
            }
        }
    }
    
    ex_logger(LOG_ERROR, "Event will not running to here!");
}

#endif