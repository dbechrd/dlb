//------------------------------------------------------------------------------
// Copyright 2018 Dan Bechard
//------------------------------------------------------------------------------

//-- header --------------------------------------------------------------------
#ifndef DLB_VECTOR_H
#define DLB_VECTOR_H

typedef struct dlb_vec__hdr {
    size_t len;
    size_t cap;
} dlb_vec__hdr;

#define dlb_vec__hdr(b) ((dlb_vec__hdr *)((char *)b - sizeof(dlb_vec__hdr)))

#define dlb_vec_len(b) ((b) ? dlb_vec__hdr(b)->len : 0)
#define dlb_vec_cap(b) ((b) ? dlb_vec__hdr(b)->cap : 0)
#define dlb_vec_end(b) ((b) + dlb_vec_len(b))
#define dlb_vec_last(b) (&(b)[dlb_vec__hdr(b)->len-1])
//#define dlb_vec_sizeof(b) ((b) ? dlb_vec_len(b) * sizeof(*(b)) : 0)
#define dlb_vec_reserve(b, n) \
    ((n) <= dlb_vec_cap(b) ? 0 : ((b) = dlb_vec__grow((b), (n), sizeof(*(b)))))
#define dlb_vec_push(b, ...) \
    (dlb_vec_reserve((b), 1 + dlb_vec_len(b)), \
    ((b)[dlb_vec__hdr(b)->len++] = (__VA_ARGS__)), \
    dlb_vec_last(b))
#define dlb_vec_alloc(b) \
    (dlb_vec_reserve((b), 1 + dlb_vec_len(b)), \
    dlb_vec__hdr(b)->len++, \
    dlb_vec_last(b))
#define dlb_vec_pop(b) (dlb_vec_len(b) > 0 ? dlb_vec__hdr(b)->len-- : 0)
#define dlb_vec_clear(b) (dlb_vec_len(b) > 0 ? dlb_vec__hdr(b)->len = 0 : 0)
#define dlb_vec_free(b) ((b) ? (free(dlb_vec__hdr(b)), (b) = NULL) : 0)

void *dlb_vec__grow(const void *buf, size_t len, size_t size);

#endif
//-- end of header -------------------------------------------------------------

//-- implementation ------------------------------------------------------------
#ifdef DLB_VECTOR_IMPLEMENTATION
#ifndef DLB_VECTOR_IMPLEMENTATION_DEF
#define DLB_VECTOR_IMPLEMENTATION_DEF

#include "dlb_types.h"
#define DLB_MEMORY_IMPLEMENTATION
#include "dlb_memory.h"
#include <string.h>

void *dlb_vec__grow(const void *buf, size_t len, size_t size) {
    DLB_ASSERT(dlb_vec_cap(buf) <= (SIZE_MAX - 1) / 2);
    size_t capacity = MAX(16, MAX(2 * dlb_vec_cap(buf), len));
    DLB_ASSERT(len <= capacity);
    DLB_ASSERT(capacity <= (SIZE_MAX - sizeof(dlb_vec__hdr))/size);
    size_t new_size = sizeof(dlb_vec__hdr) + capacity * size;
    dlb_vec__hdr *vec;
    if (buf) {
        vec = dlb_realloc(dlb_vec__hdr(buf), new_size);
        size_t old_size = sizeof(dlb_vec__hdr) + vec->cap * size;
        memset((char *)vec + old_size, 0, new_size - old_size);
    } else {
        vec = dlb_calloc(1, new_size);
        vec->len = 0;
    }
    vec->cap = capacity;
    char *new_buf = (char *)vec + sizeof(dlb_vec__hdr);
    return new_buf;
}

#endif
#endif

//-- tests ---------------------------------------------------------------------
#ifdef DLB_VECTOR_TEST
#ifndef DLB_VECTOR_TEST_DEF
#define DLB_VECTOR_TEST_DEF

static void *dlb_vec_test()
{
    int *store = NULL;
    for (int i = 0; i < 1024; i++) {
        dlb_vec_push(store, i);
    }
    for (size_t i = 0; i < dlb_vec_len(store); i++) {
        assert(store[i] == i);
    }
    dlb_vec_free(store);
}

#endif
#endif
