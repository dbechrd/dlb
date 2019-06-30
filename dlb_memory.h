//------------------------------------------------------------------------------
// Copyright 2018 Dan Bechard
//------------------------------------------------------------------------------

//-- header --------------------------------------------------------------------
#ifndef DLB_MEMORY_H
#define DLB_MEMORY_H

#include "dlb_types.h"

void *dlb_malloc(u32 size);
void *dlb_calloc(u32 count, u32 size);
void *dlb_realloc(void *block, u32 size);
void dlb_free(void *block);
void dlb_memcpy(void *dst, const void *src, u32 size);
void dlb_memset(void *dst, u8 val, u32 size);

#endif
//-- end of header -------------------------------------------------------------

//-- implementation ------------------------------------------------------------
#ifdef DLB_MEMORY_IMPLEMENTATION
#ifndef DLB_MEMORY_IMPLEMENTATION_DEF
#define DLB_MEMORY_IMPLEMENTATION_DEF

#include <stdlib.h>

void *dlb_malloc(u32 size)
{
    void *block = malloc(size);
    if (!block)
    {
        DLB_ASSERT(!"dlb_malloc error");
    }
    return block;
}

void *dlb_calloc(u32 count, u32 size)
{
    void *block = calloc(count, size);
    if (!block)
    {
		DLB_ASSERT(!"dlb_malloc error");
    }
    return block;
}

void *dlb_realloc(void *block, u32 size)
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

void dlb_memcpy(void *dst, const void *src, u32 size)
{
    for (size_t i = 0; i < size; i++) {
        ((u8 *)dst)[i] = ((u8 *)src)[i];
    }
}

void dlb_memset(void *dst, u8 val, u32 size)
{
    for (size_t i = 0; i < size; i++) {
        ((u8 *)dst)[i] = val;
    }
}

#endif
#endif