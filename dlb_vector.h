#ifndef DLB_VECTOR_H
#define DLB_VECTOR_H
//------------------------------------------------------------------------------
// Copyright 2018 Dan Bechard
//------------------------------------------------------------------------------

//-- header --------------------------------------------------------------------
#include "dlb_memory.h"

typedef struct dlb_vec__hdr {
    size_t len;        // current # of elements
    size_t cap;        // capacity in # of elements
    size_t elem_size;  // size of each element
    size_t fixed;      // 0 = dynamic allocation (min 16, 2x resize), 1 = fixed-size array (assert/no-op on resize)
} dlb_vec__hdr;

#define dlb_vec_hdr(b) ((b) ? ((dlb_vec__hdr *)b - 1) : 0)
#define dlb_vec_len(b) ((b) ? dlb_vec_hdr(b)->len : 0)
#define dlb_vec_cap(b) ((b) ? dlb_vec_hdr(b)->cap : 0)
#define dlb_vec_elem_size(b) ((b) ? dlb_vec_hdr(b)->elem_size : 0)
#define dlb_vec_fixed(b) ((b) ? dlb_vec_hdr(b)->fixed : 0)

#define dlb_vec_size(b) ((b) ? dlb_vec_len(b) * dlb_vec_elem_size(b) : 0)
#define dlb_vec_end(b) ((b) + dlb_vec_len(b))
#define dlb_vec_end_size(b, s) (void *)((char *)(b) + dlb_vec_len(b) * s)
#define dlb_vec_each(t, i, b) for (t (i) = (b); (i) != dlb_vec_end(((t)b)); (i)++)
#define dlb_vec_range(t, i, s, e) for (t (i) = (s); (i) != (e); (i)++)
#define dlb_vec_last(b) (dlb_vec_len(b) ? &(b)[dlb_vec_len(b) - 1] : 0)
#define dlb_vec_last_size(b, s) (dlb_vec_len(b) ? (void *)((char *)(b) + (s) * (dlb_vec_len(b) - 1)) : 0)
#define dlb_vec_index_size(b, i, s) (dlb_vec_len(b) ? (void *)((char *)(b) + (s) * (i)) : 0)
#define dlb_vec_reserved_bytes(b) ((b) ? dlb_vec_cap(b) * dlb_vec_elem_size(b) : 0)
#define dlb_vec_reserve(b, n) \
    ((n) <= dlb_vec_cap(b) ? 0 : ((b) = dlb_vec__grow((b), (n), sizeof(*(b)), 0)))
#define dlb_vec_reserve_size(b, n, s) \
    ((n) <= dlb_vec_cap(b) ? 0 : ((b) = dlb_vec__grow((b), (n), (s), 0)))
#define dlb_vec_reserve_fixed(b, n) \
    ((n) <= dlb_vec_cap(b) ? 0 : ((b) = dlb_vec__grow((b), (n), sizeof(*(b)), 1)))
#define dlb_vec_push(b, v) \
    (dlb_vec_reserve((b), 1 + dlb_vec_len(b)), \
    ((b)[dlb_vec_hdr(b)->len++] = (v)), \
    dlb_vec_last(b))

#define dlb_vec_alloc(b) \
    (dlb_vec_reserve((b), 1 + dlb_vec_len(b)), \
    dlb_vec_hdr(b)->len++, \
    dlb_vec_last(b))
#define dlb_vec_alloc_size(b, s) \
    (dlb_vec_reserve_size((b), 1 + dlb_vec_len(b), (s)), \
    dlb_vec_hdr(b)->len++, \
    dlb_vec_last_size(b, s))
#define dlb_vec_alloc_count(b, n) \
    (dlb_vec_reserve((b), n + dlb_vec_len(b)), \
    dlb_vec_hdr(b)->len += n)
#define dlb_vec_alloc_count_size(b, n, s) \
    (dlb_vec_reserve_size((b), n + dlb_vec_len(b), (s)), \
    dlb_vec_hdr(b)->len += n)

// Pop & return pointer to last element, returns 0 if empty
// TODO: Why would you return pointer to something that was just removed? It should return
// a copy of the object, or force the caller to use dlb_vec_last() before calling this.
#define dlb_vec_pop(b) \
    (dlb_vec_len(b) > 0 ? \
        dlb_vec_hdr(b)->len--, \
        (&(b)[dlb_vec_len(b)]) \
    : 0)
// Pop & zero last element, returns true on success, false if empty
#define dlb_vec_popz(b) \
    (dlb_vec_len(b) > 0 ? \
        (dlb_memset(dlb_vec_last(b), 0, sizeof(*(b))), \
        dlb_vec_hdr(b)->len--, \
        1) \
    : 0)
#define dlb_vec_clear(b) (dlb_vec_len(b) > 0 ? dlb_vec_hdr(b)->len = 0 : 0)
#define dlb_vec_zero(b) \
    (dlb_vec_cap(b) > 0 ? \
        (dlb_memset(b, 0, dlb_vec_reserved_bytes(b)), \
        dlb_vec_hdr(b)->len = 0) \
    : 0)
#define dlb_vec_free(b) ((b) ? (dlb_free(dlb_vec_hdr(b)), (b) = NULL) : 0)

// NOTE: This will obviously resize the buffer, so it's not really const, but if I remove the const from the decl then
// for some reason MSVC whines about all of the dlb_vec_push calls that operate on `const char **` vectors. *shrugs*
void *dlb_vec__grow(const void *buf, size_t len, size_t elem_size, size_t fixed);

#endif
//-- end of header -------------------------------------------------------------

#ifdef __INTELLISENSE__
/* This makes MSVC intellisense work. */
#define DLB_VECTOR_IMPLEMENTATION
#endif

//-- implementation ------------------------------------------------------------
#ifdef DLB_VECTOR_IMPLEMENTATION
#ifndef DLB_VECTOR_IMPL_INTERNAL
#define DLB_VECTOR_IMPL_INTERNAL

#include "dlb_types.h"
#define DLB_MEMORY_IMPLEMENTATION
#include "dlb_memory.h"
#include <assert.h>

void *dlb_vec__grow(const void *buf, size_t len, size_t elem_size, size_t fixed) {
    dlb_vec__hdr *hdr = dlb_vec_hdr(buf);
    if (hdr && hdr->fixed) {
        assert(!hdr->fixed);  // don't allow resize of fixed arrays
        // TODO: Make this safer in release mode; this just returns the same buffer with no resize
        return (void *)buf;
    }
    size_t cap = dlb_vec_cap(buf);
    assert(cap <= (SIZE_MAX - 1) / 2);
    size_t new_cap = MAX(!fixed * 16, MAX(2 * cap, len));
    assert(len <= new_cap);
    assert(new_cap);
    assert(new_cap <= (SIZE_MAX - sizeof(dlb_vec__hdr))/elem_size);
    size_t new_size = sizeof(dlb_vec__hdr) + new_cap * elem_size;
    if (hdr) {
        size_t old_size = sizeof(dlb_vec__hdr) + hdr->cap * elem_size;
        hdr = dlb_realloc(hdr, new_size);
        dlb_memset((char *)hdr + old_size, 0, new_size - old_size);
    } else {
        hdr = dlb_calloc(1, new_size);
        hdr->len = 0;
        hdr->fixed = fixed;
    }
    hdr->cap = new_cap;
    hdr->elem_size = elem_size;
    char *new_buf = (char *)hdr + sizeof(dlb_vec__hdr);
    assert(new_buf);
    return new_buf;
}

#endif
#endif
//-- end of implementation -----------------------------------------------------

//-- tests ---------------------------------------------------------------------
#ifdef DLB_VECTOR_TEST

static void *dlb_vec_test()
{
    int *store = NULL;
    for (int i = 0; i < 1024; i++) {
        dlb_vec_push(store, i);
    }
    assert(dlb_vec_len(store) == 1024);
    for (u32 i = 0; i < dlb_vec_len(store); i++) {
        assert(store[i] == i);
    }
    dlb_vec_free(store);
}

#endif
//-- end of tests --------------------------------------------------------------

#if 0
//-- notes ---------------------------------------------------------------------

// Consider instrusive array for the first 16 elements if cache is bottleneck?
typedef struct cc_vector {
    void *begin;         // dlb_vec
    void *end;           // dlb_vec
    size_t capacity;     // current capacity
    char buffer[16];     // C++: sizeof(T) * count
} cc_vector;

//-- end of tests --------------------------------------------------------------
#endif
