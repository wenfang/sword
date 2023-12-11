#include "xstr.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    unsigned int  len;
    unsigned int  size;
    char          data[];
} _xstring_hdr;

static xstring_t _newlen(const void* init, size_t initlen);
static void _clean(xstring_t s);

static xstring_t _newlen(const void* init, size_t initlen) {
    _xstring_hdr* sh;
    sh = xmem.malloc(sizeof(_xstring_hdr) + initlen + 1);
    if (!sh) {
        return NULL;
    }

    if (init && initlen) {
        memcpy(sh->data, init, initlen);
    }

    sh->len = initlen;
    sh->size = initlen;
    sh->data[sh->len] = '\0';
    return (char*)sh->data;
}

static xstring_t _new(const char* init) {
    size_t initlen = (init == NULL) ? 0 : strlen(init);
    return _newlen(init, initlen);
}

static size_t _len(xstring_t s) {
    _xstring_hdr* sh = (void*)(s - sizeof(_xstring_hdr));
    return sh->len;
}

static xstring_t _empty(void) {
    return _newlen("", 0);
}

static xstring_t _dup(xstring_t s) {
    return _newlen(s, _len(s));
}

static void _free(xstring_t s) {
    if (s == NULL) return;
    xmem.free(s - sizeof(_xstring_hdr));
}

static xstring_t _makeroom(xstring_t s, size_t addlen) {
    _xstring_hdr* sh = (void*)(s - sizeof(_xstring_hdr));
    size_t free = sh->size - sh->len;
    if (free >= addlen) return s;

    size_t newsize = 25 * (_len(s) + addlen) / 16;
    _xstring_hdr* newsh = xmem.realloc(sh, sizeof(_xstring_hdr) + newsize + 1);
    if (!newsh) {
        return NULL;
    }
    newsh->size = newsize;
    return (char*)newsh->data;
}

static xstring_t _catlen(xstring_t s, const void* t, size_t len) {
    s = _makeroom(s, len);
    if (!s) {
        return NULL;
    }
    _xstring_hdr* sh = (void*)(s - sizeof(_xstring_hdr));
    memcpy(s + sh->len, t, len);
    sh->len += len;
    sh->data[sh->len] = '\0';
    return s;
}

static xstring_t _cat(xstring_t s, const char* t) {
    return _catlen(s, t, strlen(t));
}

static xstring_t _catxs(xstring_t s, xstring_t t) {
    return _catlen(s, t, _len(t));
}

static xstring_t _catfd(xstring_t s, int fd, unsigned len, int* res) {
    s = _makeroom(s, len);
    if (!s) {
        return NULL;
    }
    _xstring_hdr* sh = (void*)(s - sizeof(_xstring_hdr));
    *res = read(fd, s + sh->len, len);
    if (*res <= 0) {
        sh->data[sh->len] = '\0';
        return s;
    }
    sh->len += *res;
    sh->data[sh->len] = '\0';
    return s;
}

static xstring_t _catvprintf(xstring_t s, const char* fmt, va_list ap) {
    va_list cpy;
    char staticbuf[1024], *buf = staticbuf;
    size_t buflen = strlen(fmt) * 2;
    if (buflen > sizeof(staticbuf)) {
        buf = xmem.malloc(buflen);
        if (!buf) {
            return NULL;
        }
    } else {
        buflen = sizeof(staticbuf);
    }
    while (1) {
        buf[buflen - 2] = '\0';
        va_copy(cpy, ap);
        vsnprintf(buf, buflen, fmt, cpy);
        va_end(cpy);
        if (buf[buflen - 2] == '\0') break;
        if (buf != staticbuf) xmem.free(buf);
        buflen *= 2;
        buf = xmem.malloc(buflen);
    }
    xstring_t t = _cat(s, buf);
    if (buf != staticbuf) xmem.free(buf);
    return t;
}

static xstring_t _catprintf(xstring_t s, const char* fmt, ...) {
    va_list ap;
    xstring_t t;
    va_start(ap, fmt);
    t = _catvprintf(s, fmt, ap);
    va_end(ap);
    return t;
}

static xstring_t _cpylen(xstring_t s, const void* t, size_t len) {
    _xstring_hdr* sh = (void*)(s - sizeof(_xstring_hdr));
    if (sh->size < len) {
        s = _makeroom(s, len - sh->len);
        sh = (void*)(s - sizeof(_xstring_hdr));
    }
    memcpy(s, t, len);
    sh->len = len;
    sh->data[sh->len] = '\0';
    return s;
}

static xstring_t _cpy(xstring_t s, const char* t) {
    return _cpylen(s, t, strlen(t));
}

static xstring_t _cpyxs(xstring_t s, xstring_t t) {
    return _cpylen(s, t, _len(t));
}

static xstring_t _cpyfd(xstring_t s, int fd, unsigned len, int* res) {
    _xstring_hdr* sh = (void*)(s - sizeof(_xstring_hdr));
    if (sh->size < len) {
        s = _makeroom(s, len - sh->len);
        sh = (void*)(s - sizeof(_xstring_hdr));
    }
    *res = read(fd, s, len);
    if (*res <= 0) {
        _clean(s);
        return s;
    }
    sh->len = *res;
    sh->data[sh->len] = '\0';
    return s;
}

static xstring_t _cpyvprintf(xstring_t s, const char* fmt, va_list ap) {
    va_list cpy;
    char staticbuf[1024], *buf = staticbuf;
    size_t buflen = strlen(fmt) * 2;
    if (buflen > sizeof(staticbuf)) {
        buf = xmem.malloc(buflen);
        if (!buf) {
            return NULL;
        }
    } else {
        buflen = sizeof(staticbuf);
    }
    while (1) {
        buf[buflen - 2] = '\0';
        va_copy(cpy, ap);
        vsnprintf(buf, buflen, fmt, cpy);
        va_end(cpy);
        if (buf[buflen - 2] == '\0') break;
        if (buf != staticbuf) xmem.free(buf);
        buflen *= 2;
        buf = xmem.malloc(buflen);
    }
    xstring_t t = _cpy(s, buf);
    if (buf != staticbuf) xmem.free(buf);
    return t;
}

static xstring_t _cpyprintf(xstring_t s, const char* fmt, ...) {
    va_list ap;
    xstring_t t;
    va_start(ap, fmt);
    t = _cpyvprintf(s, fmt, ap);
    va_end(ap);
    return t;
}

static void _clean(xstring_t s) {
    _xstring_hdr* sh = (void*)(s - sizeof(_xstring_hdr));
    sh->len = 0;
    sh->data[sh->len] = '\0';
}

static void _strim(xstring_t s, const char* cset) {
    _xstring_hdr* sh = (void*)(s - sizeof(_xstring_hdr));
    char* start, *end, *sp, *ep;
    size_t len;
    sp = start = s;
    ep = end = s + _len(s) - 1;
    while (sp <= end && strchr(cset, *sp)) sp++;
    while (ep > start && strchr(cset, *ep)) ep--;
    len = (sp > ep) ? 0 : ((ep - sp) + 1);
    if (sh->data != sp) memmove(sh->data, sp, len);
    sh->len = len;
    sh->data[sh->len] = '\0';
}

static void _range(xstring_t s, int start, int end) {
    _xstring_hdr* sh = (void*)(s - sizeof(_xstring_hdr));
    size_t newlen, len = _len(s);
    if (len == 0) return;
    if (start < 0) {
        start = len + start;
        if (start < 0) start = 0;
    }
    if (end < 0) {
        end = len + end;
        if (end < 0) end = 0;
    }
    newlen = (start > end) ? 0 : (end - start) + 1;
    if (newlen != 0) {
        if (start >= (signed)len) {
            newlen = 0;
        } else if (end >= (signed)len) {
            end = len - 1;
            newlen = (start > end) ? 0 : (end - start) + 1;
        }
    } else {
        start = 0;
    }
    if (start && newlen) memmove(sh->data, sh->data + start, newlen);
    sh->data[newlen] = 0;
    sh->len = newlen;
}

static int _search(xstring_t s, const char* key) {
    ASSERT(key);
    if (!_len(s)) return XLIB_ERR;
    char* point = strstr(s, key);
    if (!point) return XLIB_ERR;
    return point - s;
}

static xstring_t* _split(xstring_t s, const char* sep, int* count) {
    int start = 0, slots = 5;
    size_t total = _len(s);
    xstring_t* token = xmem.malloc(sizeof(xstring) * slots);
    *count = 0;
    while (start < total) {
        char* point = strstr(s + start, sep);
        if (!point) break;
        if (point != s + start) {
            token[*count] = _newlen(s + start, point - s - start);
            (*count)++;
            if (*count >= slots) {
                slots = slots * 3 / 2;
                token = xmem.realloc(token, sizeof(xstring) * slots);
            }
        }
        start = point - s + strlen(sep);
    }
    if (total != start) {
        token[*count] = _newlen(s + start, total - start);
        (*count)++;
        if (*count >= slots) {
            slots = slots * 3 / 2;
            token = xmem.realloc(token, sizeof(xstring) * slots);
        }
    }
    return token;
}

static void _freeList(xstring_t* s, int count) {
    int i;
    for (i = 0; i < count; i++) {
        _free(s[i]);
    }
    xmem.free(s);
}