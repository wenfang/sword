#include "xtask.h"
#include "xmem.h"

#include <string.h>

static LIST_HEAD(_head);
static ucontext_t   _context;
static xtask_t*     _curr;

#define XTASK_FREE  0
#define XTASK_READY 1
#define XTASK_DONE  2

static void _schedule(void);

static void _taskMain(void *arg) {
    xtask_t* task = arg;
    task->_fun(task->_arg);
    task->_status = XTASK_DONE;
    _schedule();
}

static xtask_t* _create(void (*fun)(void*), void* arg, size_t size) {
    // 栈不能小于1K
    if (size < 1024) {
        return NULL; 
    }
    xtask_t* task = xmem.malloc(sizeof(xtask_t) + size);
    if (task == NULL) {
        return NULL;
    }
    memset(task, 0, sizeof(task));

    task->_stk = (void *)(task + 1);
    task->_size = size;
    task->_fun = fun;
    task->_arg = arg;
    task->_status = XTASK_FREE;

    INIT_LIST_HEAD(&task->_node);
    getcontext(&task->_context);
    task->_context.uc_stack.ss_sp = task->_stk + 8;
    task->_context.uc_stack.ss_size = task->_size - 64;
    makecontext(&task->_context, (void (*)(void))_taskMain, 1, task);
    return task;
}

static xtask_t* _current(void) {
    return _curr;
}

static void _ready(xtask_t* task) {
    if (task->_status == XTASK_READY) {
        return;
    }
    list_add_tail(&task->_node, &_head);
    task->_status = XTASK_READY;
}

static void _schedule(void) {
    swapcontext(&_curr->_context, &_context);
}

static void _process(void) {
    while (!list_empty(&_head)) {
        xtask_t *task = list_first_entry(&_head, xtask_t, _node);
        if (!task) {
            break;
        }
        list_del_init(&task->_node);
        task->_status = XTASK_FREE;
        _curr = task;
        swapcontext(&_context, &task->_context);
        if (task->_status == XTASK_DONE) {
            xmem.free(task);
        }
    }
}

xtask_p xtask = { 
    _create, 
    _current,
    _ready, 
    _schedule, 
    _process 
};
