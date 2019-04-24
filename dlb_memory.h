//------------------------------------------------------------------------------
// Copyright 2018 Dan Bechard
//------------------------------------------------------------------------------

//-- header --------------------------------------------------------------------
#ifndef DLB_MEMORY_H
#define DLB_MEMORY_H

#include <stdlib.h>

void *dlb_malloc(size_t size);
void *dlb_calloc(size_t count, size_t size);
void *dlb_realloc(void *block, size_t size);
void dlb_free(void *block);

#endif
//-- end of header -------------------------------------------------------------

//-- implementation ------------------------------------------------------------
#ifdef DLB_MEMORY_IMPLEMENTATION
#ifndef DLB_MEMORY_IMPLEMENTATION_DEF
#define DLB_MEMORY_IMPLEMENTATION_DEF

#include "dlb_types.h"

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

#endif
#endif