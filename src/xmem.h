#ifndef __XMEM_H
#define __XMEM_H

#include <stdio.h>

typedef struct {
	void* (*malloc)(size_t size);
	void* (*calloc)(size_t size);
	void* (*realloc)(void* ptr, size_t size);
	void  (*free)(void* ptr);
} xmem_p;

extern xmem_p xmem;

#endif
