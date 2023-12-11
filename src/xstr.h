#ifndef XSTR_H
#define XSTR_H

#include <stdio.h>
#include <stdarg.h>

typedef char* xstr_t;

xstr_t xstr_new(const char* init);
size_t xstr_len(xstr_t s);
xstr_t xstr_empty(void);
xstr_t xstr_dup(xstr_t s);
void xstr_free(xstr_t s);
xstr_t xstr_makeroom(xstr_t s, size_t addlen);

xstr_t xstr_cat(xstr_t s, const char* t);
xstr_t xstr_catx(xstr_t s, xstr_t t);
xstr_t xstr_catprintf(xstr_t s, const char* fmt, ...);

xstr_t xstr_cpy(xstr_t s, const char* t);
xstr_t xstr_cpyx(xstr_t s, xstr_t t);
xstr_t xstr_cpyprintf(xstr_t s, const char* fmt, ...);

void xstr_clean(xstr_t s);
void xstr_strim(xstr_t s, const char* cset);

void xstr_range(xstr_t s, int start, int end);

int xstr_search(xstr_t s, const char* key);

#endif
