#ifndef DLB_MEMORY_H
#define DLB_MEMORY_H
//------------------------------------------------------------------------------
// Copyright 2018 Dan Bechard
//------------------------------------------------------------------------------

//-- header --------------------------------------------------------------------
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#if _DEBUG
#define DLB_MALLOC(size) dlb_malloc_dbg(size, __FILE__, __LINE__)
#define DLB_CALLOC(size) dlb_calloc_dbg(size, __FILE__, __LINE__)
#define DLB_REALLOC(size) dlb_realloc_dbg(size, __FILE__, __LINE__)
#define DLB_FREE(size) dlb_free_dbg(size, __FILE__, __LINE__)
#else
#define DLB_MALLOC(size) dlb_malloc(size)
#define DLB_CALLOC(size) dlb_calloc(size)
#define DLB_REALLOC(size) dlb_realloc(size)
#define DLB_FREE(size) dlb_free(size)
#endif

static inline void *dlb_malloc(size_t size)
{
    void *block = malloc(size);
    if (!block) {
        assert(!"dlb_malloc error");
        exit(-404);
    }
    return block;
}

static inline void *dlb_calloc(size_t count, size_t size)
{
    void *block = calloc(count, size);
    if (!block) {
        assert(!"dlb_calloc error");
        exit(-404);
    }
    return block;
}

static inline void *dlb_realloc(void *block, size_t size)
{
    void *block_new = realloc(block, size);
    if (!block_new) {
        assert(!"dlb_realloc error");
        exit(-404);
    }
    return block_new;
}

static inline void dlb_free(void *block)
{
    if (block) {
        free(block);
    }
}


static inline void *dlb_malloc_dbg(size_t size, const char *file, int line)
{
    void *block = _malloc_dbg(size, );
    if (!block) {
        assert(!"dlb_malloc error");
        exit(-404);
    }
    return block;
}

static inline void *dlb_calloc_dbg(size_t count, size_t size, const char *file, int line)
{
    void *block = calloc(count, size);
    if (!block) {
        assert(!"dlb_calloc error");
        exit(-404);
    }
    return block;
}

static inline void *dlb_realloc_dbg(void *block, size_t size, const char *file, int line)
{
    void *block_new = realloc(block, size);
    if (!block_new) {
        assert(!"dlb_realloc error");
        exit(-404);
    }
    return block_new;
}

static inline void dlb_free_dbg(void *block, const char *file, int line)
{
    if (block) {
        free(block);
    }
}

static inline void dlb_memcpy(void *dst, const void *src, size_t size)
{
    const u8 *s = (u8 *)src;
    u8 *d = (u8 *)dst;
    assert(!(d > s && d < s + size));  // Overwriting src! Use dlb_memmove.
    for (size_t i = 0; i < size; ++i) {
        d[i] = s[i];
    }
}

static inline void dlb_memmove(void *dst, const void *src, size_t size)
{
    const u8 *s = (u8 *)src;
    u8 *d = (u8 *)dst;
    if (d > s && d < s + size) {
        // Iterate backwards when dst overwrites src bytes we haven't copied yet
        for (size_t i = size - 1; i > 0; --i) {
            d[i] = s[i];
        }
        d[0] = s[0];
    } else {
        for (size_t i = 0; i < size; ++i) {
            d[i] = s[i];
        }
    }
}

static inline void dlb_memset(void *dst, char val, size_t size)
{
#if 1
    // ~25% faster in debug, doesn't seem to matter in release
    memset(dst, val, size);
#else
    //for (size_t i = 0; i < size; i++) {
    //    ((u8 *)dst)[i] = val;
    //}
#endif
}

#endif
//-- end of header -------------------------------------------------------------
