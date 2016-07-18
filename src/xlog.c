#include "xlog.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>

#define MAX_MSG_LEN 1024

static int outFd = STDOUT_FILENO;
static int errFd = STDERR_FILENO;

static void _open(const char* outFile, const char* errFile) {
    int newOutFd, newErrFd;

    newOutFd = open(outFile, O_WRONLY | O_APPEND | O_CREAT, 0666);
    if (newOutFd >= 0) {
        if (outFd != STDOUT_FILENO) {
            close(outFd);
        }
        outFd = newOutFd;
    }

    newErrFd = open(errFile, O_WRONLY | O_APPEND | O_CREAT, 0666);
    if (newErrFd >= 0) {
        if (errFd != STDERR_FILENO) {
            close(errFd);
        }
        errFd = newErrFd;
    }
}

static void _out(const char* message, ...) {
    char msg[MAX_MSG_LEN + 1];
    va_list ap;

    va_start(ap, message);
    vsnprintf(msg, MAX_MSG_LEN, message, ap);
    va_end(ap);

    write(outFd, msg, strlen(msg));
    write(errFd, msg, strlen(msg));
}

static void _err(const char* message, ...) {
    char msg[MAX_MSG_LEN + 1];
    va_list ap;

    va_start(ap, message);
    vsnprintf(msg, MAX_MSG_LEN, message, ap);
    va_end(ap);

    write(errFd, msg, strlen(msg));
}

static void _close(void) {
    if (outFd != STDOUT_FILENO) close(outFd);
    if (errFd != STDERR_FILENO) close(errFd);
}

xlog_p xlog = {
    _open,
    _out,
    _err,
    _close
};
