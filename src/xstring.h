#ifndef __XSTRING_H
#define __XSTRING_H

#include <stdio.h>
#include <stdarg.h>

typedef char* xstring_t;

typedef struct {
    xstring_t	(*newlen)(const void* init, size_t initlen);
    xstring_t 	(*new)(const char* init);
    size_t     	(*len)(xstring_t s);
    xstring_t	(*empty)(void);
    xstring_t   (*dup)(xstring_t s);
    void        (*free)(xstring_t s);
    xstring_t   (*makeroom)(xstring_t s, size_t addlen);

    xstring_t   (*catlen)(xstring_t s, const void* t, size_t len);
    xstring_t   (*cat)(xstring_t s, const char* t);
    xstring_t   (*catxs)(xstring_t s, xstring_t t);
    xstring_t   (*catfd)(xstring_t s, int fd, unsigned len, int* res);
    xstring_t   (*catprintf)(xstring_t s, const char* fmt, ...);

    xstring_t   (*cpylen)(xstring_t s, const void* t, size_t len);
    xstring_t   (*cpy)(xstring_t s, const char* t);
    xstring_t   (*cpyxs)(xstring_t s, xstring_t t);
    xstring_t   (*cpyfd)(xstring_t s, int fd, unsigned len, int* res);
    xstring_t   (*cpyprintf)(xstring_t s, const char* fmt, ...);

    void        (*clean)(xstring_t s);
    void        (*strim)(xstring_t s, const char* cset);

    void        (*range)(xstring_t s, int start, int end);

    int         (*search)(xstring_t s, const char* key);

    xstring_t*	(*split)(xstring_t s, const char* sep, int* count);
    void        (*freeList)(xstring_t* s, int count);
} xstring_p;

extern const xstring_p xstring;

#endif
