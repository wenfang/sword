#ifndef __XIO_H
#define __XIO_H

#include "xstring.h"

#define XIO_CLOSED 0
#define XIO_ERROR  -1

typedef struct xio_s xio_t;

typedef struct {
    xio_t* (*new)(const char* fname, int create);
    xio_t* (*newfd)(int fd);
    void   (*free)(xio_t* io);

    int (*read)(xio_t* io, xstring_t* s);
    int (*write)(xio_t* io, xstring_t s);
    int (*flush)(xio_t* io);
} xio_p;

extern xio_p xio;

#endif
