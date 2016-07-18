#ifndef __XUTIL_H
#define __XUTIL_H

#include "xcommon.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>

#define likely(x)   __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)

#ifdef DEBUG
static inline void
_Assert(char* file, unsigned line) {
    fprintf(stderr, "Assert Failed [%s:%d]\n", file, line);
    abort();
}
#define ASSERT(x) \
    if (!(x)) _Assert(__FILE__, __LINE__)
#else
#define ASSERT(x)
#endif

typedef struct {
    char timeString[32];

    int             (*daemon)(void);
    unsigned        (*cpuNum)(void);
    int             (*setMaxOpenFile)(unsigned fileNum);
    unsigned long   (*currentTime)(void);
    void            (*updateTime)(void);

    pid_t   (*getPid)(const char* pidFile);
    int     (*savePid)(const char* pidFile);
    int     (*removePid)(const char* pidFile);

    void (*initTitle)(int argc, char** argv);
    void (*setTitle)(const char* title);
} xutil_p;

extern xutil_p xutil;

#endif
