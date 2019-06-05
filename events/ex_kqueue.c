/**
 * Copyright @2019 Exserver All Rights Reserved.
 */

#include <ex_types.h>
#include <ex_log.h>
#include <ex_string.h>
#include <ex_kqueue.h>

#ifndef __linux__

inline int ex_init_events()
{
    int fd;
    
    fd = kqueue();
    if ( fd == -1 )
    {
        ex_logger(LOG_ERROR, "Create kqueue fd failed; " EX_ERROR_MSG_FMT, EX_ERROR_MSG_STR);
        return -1;
    }
    return fd;
}

inline int ex_add_to_events(int fd, int sd)
{
    int re;
    struct kevent   ev;
    struct timespec ts;
    ex_memzero(&ev, sizeof(ev));
    ex_memzero(&ts, sizeof(ts));
    
    ts.tv_sec = ts.tv_nsec = 0;
    EV_SET(&ev, sd, EV_READ, EV_ADD, 0, 0, NULL);
    re = kevent(fd, &ev, 1, NULL, 0, &ts);
    if ( re == -1 )
    {
        ex_logger(LOG_ERROR, "Add fd %d to kqueue: %d\n" EX_ERROR_MSG_FMT, sd, fd, EX_ERROR_MSG_STR);
        return -1;
    }
    
    return 1;
}

inline int ex_del_from_events(int fd, int sd)
{
    int re;
    struct kevent   ev;
    struct timespec ts;
    ex_memzero(&ev, sizeof(ev));
    ex_memzero(&ts, sizeof(ts));
    
    ts.tv_sec = ts.tv_nsec = 0;
    EV_SET(&ev, sd, EV_WRITE, EV_DELETE, 0, 0, NULL);
    re = kevent(fd, &ev, 1, NULL, 0, &ts);
    if ( re == -1 )
    {
        ex_logger(LOG_ERROR, "Add fd %d to kqueue: %d\n" EX_ERROR_MSG_FMT, sd, fd, EX_ERROR_MSG_STR);
        return -1;
    }
    
    return 1;
}

inline void ex_event_loop(int fd, void (*func)(int, int, int))
{
    int             i, j;
    struct timespec ts;
    struct kevent   events[EVENT_SIZE];
    
    ex_memzero(&ts,    sizeof(ts));
    
    ts.tv_sec  = EVENT_TIMEOUT;
    ts.tv_nsec = 0;
    
    while (true)
    {
        ex_memzero(events, sizeof(events));
        
        i = kevent( fd, NULL, 0, events, EVENT_SIZE, &ts );
    
        if ( i == -1 )
        {
            ex_logger( LOG_ERROR, "kevent error; " EX_ERROR_MSG_FMT, EX_ERROR_MSG_STR );
            continue;
        }
        
        for (j = 0; j < i; j++)
        {
            if ( events[j].flags & ( EV_ERROR | EV_EOF ))
            {
                ex_del_from_events(fd, (int)events[j].ident);
                close( ( int )events[j].ident );
                continue;
            }
            
            if (   events[j].filter == EV_READ
                || events[j].filter == EV_WRITE )
            {
                func((int)events[j].ident, events[j].filter, fd);
            }
        }
    }
    
    ex_logger(LOG_ERROR, "Event will not running to here!");
}

#endif