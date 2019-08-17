//------------------------------------------------------------------------------
// Copyright 2018 Dan Bechard
//------------------------------------------------------------------------------

//-- header --------------------------------------------------------------------
#ifndef DLB_MEMORY_H
#define DLB_MEMORY_H

void *dlb_malloc(size_t size);
void *dlb_calloc(size_t count, size_t size);
void *dlb_realloc(void *block, size_t size);
void dlb_free(void *block);
void dlb_memcpy(void *dst, const void *src, size_t size);
void dlb_memset(void *dst, char val, size_t size);

#endif
//-- end of header -------------------------------------------------------------

//-- implementation ------------------------------------------------------------
#ifdef DLB_MEMORY_IMPLEMENTATION
#ifndef DLB_MEMORY_IMPLEMENTATION_DEF
#define DLB_MEMORY_IMPLEMENTATION_DEF

#include <stdlib.h>

void *dlb_malloc(size_t size)
{
    void *block = malloc(size);
    if (!block)
    {
        DLB_ASSERT(!"dlb_malloc error");
    }
    return block;
}

void *dlb_calloc(size_t count, size_t size)
{
    void *block = calloc(count, size);
    if (!block)
    {
		DLB_ASSERT(!"dlb_malloc error");
    }
    return block;
}

void *dlb_realloc(void *block, size_t size)
{
    void *block_new = realloc(block, size);
    if (!block_new)
    {
		DLB_ASSERT(!"dlb_malloc error");
    }
    return block_new;
}

void dlb_free(void *block)
{
	if (block)
    {
		free(block);
	}
}

void dlb_memcpy(void *dst, const void *src, size_t size)
{
    for (size_t i = 0; i < size; i++) {
        ((u8 *)dst)[i] = ((u8 *)src)[i];
    }
}

void dlb_memset(void *dst, char val, size_t size)
{
    for (size_t i = 0; i < size; i++) {
        ((u8 *)dst)[i] = val;
    }
}

#endif
#endif