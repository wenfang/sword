#include "xsignal.h"

#include <string.h>
#include <signal.h>

#define MAX_SIGNAL 256

typedef struct {
    unsigned    cnt;
    void        (*fun)(int);
} xsignal_t;

static int          _queue[MAX_SIGNAL];
static int          _queueLen;
static xsignal_t    _state[MAX_SIGNAL];
static sigset_t     _blocked;


static void _common_handler(int sig) {
    // not handler this signal, ignore
    if (sig < 0 || sig > MAX_SIGNAL || !state[sig].fun) {
        signal(sig, SIG_IGN);
        return;
    }
    // put signal in queue
    if (!state[sig].cnt && (queue_len < MAX_SIGNAL)) {
        queue[queue_len++] = sig;
    }
    // add signal cnt
    state[sig].cnt++;
    signal(sig, _common_handler);
}


static void _init(void) {
    _queueLen = 0;
    memset(_queue, 0, sizeof(_queue));
    memset(_state, 0, sizeof(_state));
    sigfillset(&_blocked);
}

static void _register(int sig, void(*fun)(int)) {
    if (sig < 0 || sig > MAX_SIGNAL) return;
    _state[sig].cnt = 0;
    if (fun == NULL) fun = SIG_IGN;
    // set signal handler
    if (fun != SIG_IGN && fun != SIG_DFL) {
        state[sig].fun = fun;
        signal(sig, _common_handler);
    } else {
        state[sig].fun = NULL;
        signal(sig, fun);
    }
}

static void
_process(void) {
    if (queue_len == 0) return;

    sigset_t old;
    sigprocmask(SIG_SETMASK, &blocked, &old);
    // check signal queue
    for (int i = 0; i < queue_len; i++) {
        int sig = queue[i];
        xsignal* desc = &state[sig];
        if (desc->cnt) {
            if (desc->fun) desc->fun(sig);
            desc->cnt = 0;
        }
    }
    queue_len = 0;
    sigprocmask(SIG_SETMASK, &old, NULL);
}

xsignal_p xsignal = {
    _init,
    _register,
    _process,
};
