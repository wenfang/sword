#include "xutil.h"
#include "xmem.h"
#include <fcntl.h>

static int _daemon() {
    switch (fork()) {
    case -1:
        return XLIB_ERR;
    case 0:   // child return here
        break;
    default:  // parent return and exit
        exit(EXIT_SUCCESS);
    }

    if (setsid() == -1) return XLIB_ERR;
    if (chdir("/") != 0) return XLIB_ERR;

    int fd;
    if ((fd = open("/dev/null", O_RDWR, 0)) == -1) return XLIB_ERR;
    if (dup2(fd, STDIN_FILENO) < 0) return XLIB_ERR;
    if (dup2(fd, STDOUT_FILENO) < 0) return XLIB_ERR;
    if (dup2(fd, STDERR_FILENO) < 0) return XLIB_ERR;
    if (fd > STDERR_FILENO) close(fd);
    return XLIB_OK;
}

static unsigned _cpuNum() {
    return sysconf(_SC_NPROCESSORS_ONLN);
}

static int _setMaxOpenFiles(unsigned fileNum) {
    struct rlimit r;

    r.rlim_cur = fileNum;
    r.rlim_max = fileNum;
    if (setrlimit(RLIMIT_NOFILE, &r) < 0) {
        return XLIB_ERR;
    }
    return XLIB_OK;
}

static unsigned long _currentTime() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

static void _updateTime(void) {
    time_t t;
    struct tm* lt;

    t = time(NULL);
    lt = localtime(&t);
    strftime(xutil.timeString, 32, "%Y-%m-%d %H:%M:%S", lt);
}

pid_t _getPid(const char* pidFile) {
    ASSERT(pidFile);

    long pid;
    FILE* fp;

    if (!(fp = fopen(pidFile, "r"))) return 0;
    fscanf(fp, "%ld\n", &pid);
    fclose(fp);
    return pid;
}

int _savePid(const char* pidFile) {
    ASSERT(pidFile);

    FILE* fp;
    if (!(fp = fopen(pidFile, "w"))) return XLIB_ERR;
    fprintf(fp, "%ld\n", (long)getpid());
    if (fclose(fp) == -1) return XLIB_ERR;
    return XLIB_OK;
}


int _removePid(const char* pidFile) {
    ASSERT(pidFile);

    if (unlink(pidFile)) return XLIB_ERR;
    return XLIB_OK;
}

extern char** environ;

static char** xargv;
static char* xargv_last = NULL;

void _initTitle(int argc, char** argv) {
    xargv = argv;

    size_t size = 0;
    for (int i = 0; environ[i]; i++) {
        size += strlen(environ[i]) + 1;
    }

    char* p = xmem.calloc(size);
    xargv_last = xargv[0];
    for (int i = 0; xargv[i]; i++) {
        if (xargv_last == xargv[i]) xargv_last = xargv[i] + strlen(xargv[i]) + 1;
    }

    for (int i = 0; environ[i]; i++) {
        if (xargv_last == environ[i]) {
            size = strlen(environ[i]) + 1;
            xargv_last = environ[i] + size;
            strncpy(p, environ[i], size);
            environ[i] = p;
            p += size;
        }
    }

    xargv_last--;
}

void _setTitle(const char* title) {
    xargv[1] = NULL;
    strncpy(xargv[0], title, xargv_last - xargv[0]);
}

xutil_p xutil = {
    "2015-06-05 11:30:00",
    _daemon,
    _cpuNum,
    _setMaxOpenFiles,
    _currentTime,
    _updateTime,
    _getPid,
    _savePid,
    _removePid,
    _initTitle,
    _setTitle
};
