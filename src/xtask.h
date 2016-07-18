#ifndef __XTASK_H
#define __XTASK_H

#include "list.h"
#include <ucontext.h>

typedef struct {
    struct list_head    _node;
    ucontext_t          _context;
    void*               _stk;
    size_t              _size;
    void                (*_fun)(void*);
    void*               _arg;
    int                 _status;
} xtask_t;

typedef struct {
    xtask_t*    (*create)(void(*fun)(void*), void* arg, size_t size);
    xtask_t*    (*current)(void);
    void        (*ready)(xtask_t *task);
    void        (*schedule)(void);
    void        (*process)(void);
} xtask_p;

extern xtask_p xtask;

#endif
