#ifndef __XEPOLL_H
#define __XEPOLL_H

#include "xtask.h"
#include <stdbool.h>

typedef struct {
    bool    (*init)(int maxfd);
    void    (*deinit)(void);
    bool    (*setRead)(unsigned fd, xtask_t* task);
    bool    (*unsetRead)(unsigned fd);
    bool    (*setWrite)(unsigned fd, xtask_t* task);
    bool    (*unsetWrite)(unsigned fd);
    void    (*process)(int timeout);
} xepoll_p;

extern xepoll_p xepoll;

#endif
