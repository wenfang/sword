#include "xio.h"
#include "xmem.h"
#include "xutil.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>

#define BUF_LEN 4096

#define XIO_READNONE     0
#define XIO_READ         1
#define XIO_READBYTES    2
#define XIO_READUNTIL    3

typedef struct {
    int         _fd;
    xstring_t   _rbuf;
    xstring_t   _wbuf;
    unsigned    _closed: 1;
    unsigned    _error: 1;
} xio_t __attribute__((aligned(sizeof(long))));

static int _read(xio_t* io, xstring_t* s) {
    ASSERT(io && s);
    if (io->_closed) return XIO_CLOSED;
    if (io->_error) return XIO_ERROR;

    int res;
    for (;;) {
        unsigned len = xstring.len(io->_rbuf);
        if (len > 0) {
            *s = xstring.catxs(*s, io->_rbuf);
            xstring.clean(io->_rbuf);
            return len;
        }

        io->_rbuf = xstring.catfd(io->_rbuf, io->_fd, BUF_LEN, &res);
        if (res < 0) {
            if (errno == EINTR) continue;
            io->_error = 1;
            break;
        }
        if (res == 0) {
            io->_closed = 1;
            break;
        }
    }
    // read error copy data
    *s = xstring.catxs(*s, io->_rbuf);
    xstring.clean(io->_rbuf);
    return res;
}

static int _write(xio_t* io, xstring_t s) {
    ASSERT(io && s);
    if (io->_closed) return XIO_CLOSED;
    if (io->_error) return XIO_ERROR;

    io->_wbuf = xstring.catxs(io->_wbuf, s);
    return xstring.len(s);
}

static int _flush(xio_t* io) {
    ASSERT(io);
    if (io->_closed) return XIO_CLOSED;
    if (io->_error) return XIO_ERROR;

    int total = 0, len = xstring.len(io->_wbuf);
    while (total < len) {
        int res = write(io->_fd, io->_wbuf + total, len - total);
        if (res < 0) {
            if (errno == EINTR) continue;
            io->_error = 1;
            break;
        }
        total += res;
    }
    xstring.range(io->_wbuf, total, -1);
    return total;
}

static xio_t* _newfd(int fd) {
    xio_t* io = xmem.calloc(sizeof(xio));
    if (io == NULL) {
        return NULL;
    }

    io->_fd   = fd;
    io->_rbuf = xstring.empty();
    io->_wbuf = xstring.empty();
    return io;
}

static xio_t* _new(const char* fname, int create) {
    ASSERT(fname);
    int fd;
    if (create) {
        fd = open(fname, O_RDWR | O_CREAT, 0666);
    } else {
        fd = open(fname, O_RDWR);
    }
    if (fd < 0) return NULL;

    return _newfd(fd);
}

static void _free(xio_t* io) {
    ASSERT(io);
    close(io->_fd);
    xstring.free(io->_rbuf);
    xstring.free(io->_wbuf);
    xmem.free(io);
}

xio_p xio = {
    _new,
    _newfd,
    _free,
    _read,
    _write,
    _flush
};
