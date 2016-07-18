#include "xepoll.h"
#include "xutil.h"
#include "xmem.h"
#include "xlog.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <errno.h>

#define XEPOLL_NONE   0
#define XEPOLL_READ   1
#define XEPOLL_WRITE  2

typedef struct {
    xtask_t*    _readTask;
    xtask_t*    _writeTask;
    unsigned    _mask;
} xepoll_t __attribute__ ((aligned(sizeof(long))));

static int                  _epfd;
static int                  _maxfd;
static xepoll_t             *_epolls;
static struct epoll_event   *_epollEvents;

static bool _init(int maxfd) {
    _epfd = epoll_create(10240);
    if (_epfd < 0) {
        return false;
    }

    _maxfd = maxfd;

    _epolls = xmem.calloc(sizeof(xepoll) * _maxfd);
    if (_epolls == NULL) {
        return false;
    }
    _epollEvents = xmem.calloc(sizeof(struct epoll_event) * maxfd);
    if (_epollEvents == NULL) {
        return false;
    }
    return true;
}

static void _deinit(void) {
    close(_epfd);
    xmem.free(_epolls);
    xmem.free(_epollEvents);
}


static bool change_epoll(unsigned fd, xepoll_t* epoll_t, unsigned newmask) {
    if (epoll_t->_mask == newmask) return true;
    // 设置epoll_event 
    struct epoll_event ee;
    ee.data.fd = fd;
    ee.events = 0;
    if (newmask & XEPOLL_READ) ee.events |= EPOLLIN;
    if (newmask & XEPOLL_WRITE) ee.events |= EPOLLOUT;
    // 设置操作类型
    int op = EPOLL_CTL_MOD;
    if (epoll_t->_mask == XEPOLL_NONE) {
        op = EPOLL_CTL_ADD;
    } else if (newmask == XEPOLL_NONE) {
        op = EPOLL_CTL_DEL;
    }
    if (epoll_ctl(_epfd, op, fd, &ee) == -1) {
        return false;
    }
    epoll_t->_mask = newmask;
    return true;
}

bool _setRead(unsigned fd, xtask_t* task) {
    if (fd >= _maxfd) {
        return false;
    }
    xepoll_t *epoll_t = &_epolls[fd];
    epoll_t->_readTask = task;
    return change_epoll(fd, epoll_t, epoll_t->_mask | XEPOLL_READ);
}

bool _unsetRead(unsigned fd) {
    if (fd >= _maxfd) {
        return false;
    }
    xepoll_t *epoll_t = &_epolls[fd];
    epoll_t->_readTask = NULL;
    return change_epoll(fd, epoll_t, epoll_t->_mask & (~XEPOLL_READ));
}

bool _setWrite(unsigned fd, xtask_t* task) {
    if (fd >= _maxfd) {
        return false;
    }
    xepoll_t *epoll_t = &_epolls[fd];
    epoll_t->_writeTask = task;
    return change_epoll(fd, epoll_t, epoll_t->_mask | XEPOLL_WRITE);
}

bool _unsetWrite(unsigned fd) {
    if (fd >= _maxfd) {
        return false;
    }
    xepoll_t *epoll_t = &_epolls[fd];
    epoll_t->_writeTask = NULL;
    return change_epoll(fd, epoll_t, epoll_t->_mask & (~XEPOLL_READ));
}

void _process(int timeout) {
    int events_n = epoll_wait(_epfd, _epollEvents, _maxfd, timeout);
    if (unlikely(events_n < 0)) {
        if (errno == EINTR)
            return;
        XLOG_ERROR("epoll_wait error: %s", strerror(errno));
        return;
    }
    if (events_n == 0) return;
    // check events
    for (int i = 0; i < events_n; i++) {
        struct epoll_event *e = &_epollEvents[i];
        xepoll_t *epoll_t = &_epolls[e->data.fd];
        int flag = 0;
        if ((e->events & EPOLLIN) && (epoll_t->_mask & XEPOLL_READ)) {
            flag = 1;
            xtask.ready(epoll_t->_readTask);
        }
        if ((e->events & EPOLLOUT) && (epoll_t->_mask & XEPOLL_WRITE)) {
            flag = 1;
            xtask.ready(epoll_t->_writeTask);
        }
        if (flag == 0) XLOG_ERROR("%d:%d %d, %d", events_n, i, e->data.fd, e->events);
    }
}

xepoll_p xepoll = {
    _init,
    _deinit,
    _setRead,
    _unsetRead,
    _setWrite,
    _unsetWrite,
    _process 
};
