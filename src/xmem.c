#include "xmem.h"

#include <stdlib.h>

static void* _malloc(size_t size) {
    void* ptr = malloc(size);
    return ptr;
}

static void* _calloc(size_t size) {
    void* ptr = calloc(1, size);
    return ptr;
}

static void* _realloc(void* ptr, size_t size) {
    void* realptr = realloc(ptr, size);
    return realptr;
}

static void _free(void* ptr) {
    free(ptr);
}

const xmem_p xmem = {
    _malloc,
    _calloc,
    _realloc,
    _free
};
