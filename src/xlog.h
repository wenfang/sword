#ifndef __XLOG_H
#define __XLOG_H

#include "xutil.h"
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <libgen.h>
#include <time.h>

#define XLOG_FATAL(fmt, arg...)   xlog.err("%-8s %s [%d:%x] [%s:%d] msg=["#fmt"]\n", "FATAL", xutil.timeString, getpid(), pthread_self(), basename(__FILE__), __LINE__, ##arg)
#define XLOG_ERROR(fmt, arg...)   xlog.err("%-8s %s [%d:%x] [%s:%d] msg=["#fmt"]\n", "ERROR", xutil.timeString, getpid(), pthread_self(), basename(__FILE__), __LINE__, ##arg)
#define XLOG_WARNING(fmt, arg...) xlog.out("%-8s %s [%d:%x] [%s:%d] msg=["#fmt"]\n", "WARNING", xutil.timeString, getpid(), pthread_self(), basename(__FILE__), __LINE__, ##arg)
#define XLOG_INFO(fmt, arg...)    xlog.out("%-8s %s [%d:%x] [%s:%d] msg=["#fmt"]\n", "INFO", xutil.timeString, getpid(), pthread_self(), basename(__FILE__), __LINE__, ##arg)
#define XLOG_DEBUG(fmt, arg...)   xlog.out("%-8s %s [%d:%x] [%s:%d] msg=["#fmt"]\n", "DEBUG", xutil.timeString, getpid(), pthread_self(), basename(__FILE__), __LINE__, ##arg)

typedef struct {
    void    (*open)(const char* outFile, const char* errFile);
    void    (*out)(const char* message, ...);
    void    (*err)(const char* message, ...);
    void    (*close)(void);
} xlog_p;

extern xlog_p xlog;

#endif
