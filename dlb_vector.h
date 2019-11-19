//------------------------------------------------------------------------------
// Copyright 2018 Dan Bechard
//------------------------------------------------------------------------------

//-- header --------------------------------------------------------------------
#ifndef DLB_VECTOR_H
#define DLB_VECTOR_H

#include "dlb_types.h"
#include "dlb_memory.h"

typedef struct dlb_vec__hdr {
    u32 len;
	u32 cap;
    u8 fixed;  // If true, fixed-size buffer that asserts when full
} dlb_vec__hdr;

#define dlb_vec_hdr(b) \
    ((b) ? ((dlb_vec__hdr *)((u8 *)b - sizeof(dlb_vec__hdr))) : 0)

#define dlb_vec_len(b) ((b) ? dlb_vec_hdr(b)->len : 0)
#define dlb_vec_used_bytes(b) ((b) ? dlb_vec_len(b) * sizeof(*(b)) : 0)
#define dlb_vec_cap(b) ((b) ? dlb_vec_hdr(b)->cap : 0)
#define dlb_vec_end(b) ((b) + dlb_vec_len(b))
#define dlb_vec_end_size(b, s) ((u8 *)(b) + dlb_vec_len(b) * s)
#define dlb_vec_each(t, i, b) for (t (i) = (b); (i) != dlb_vec_end(((t)b)); (i)++)
#define dlb_vec_range(t, i, s, e) for (t (i) = (s); (i) != (e); (i)++)
#define dlb_vec_last(b) (&(b)[dlb_vec_hdr(b)->len-1])
#define dlb_vec_last_size(b, s) (void *)((u8 *)(b) + (s) * (dlb_vec_hdr(b)->len-1))
#define dlb_vec_index_size(b, i, s) (void *)((u8 *)(b) + (s) * (i))
#define dlb_vec_reserved_bytes(b) ((b) ? dlb_vec_cap(b) * sizeof(*(b)) : 0)
#define dlb_vec_reserve(b, n) \
    ((n) <= dlb_vec_cap(b) ? 0 : ((b) = dlb_vec__grow((b), (n), sizeof(*(b)))))
#define dlb_vec_reserve_size(b, n, s) \
    ((n) <= dlb_vec_cap(b) ? 0 : ((b) = dlb_vec__grow((b), (n), (s))))
#define dlb_vec_reserve_fixed(b, n) \
    (dlb_vec_reserve(b, n), dlb_vec_hdr(b)->fixed = 1)
#define dlb_vec_fixed(b) (dlb_vec_hdr(b)->fixed)
#define dlb_vec_push(b, ...) \
    (dlb_vec_reserve((b), 1 + dlb_vec_len(b)), \
    ((b)[dlb_vec_hdr(b)->len++] = (__VA_ARGS__)), \
    dlb_vec_last(b))
#define dlb_vec_alloc(b) \
    (dlb_vec_reserve((b), 1 + dlb_vec_len(b)), \
    dlb_vec_hdr(b)->len++, \
    dlb_vec_last(b))
#define dlb_vec_alloc_size(b, s) \
    (dlb_vec_reserve_size((b), 1 + dlb_vec_len(b), (s)), \
    dlb_vec_hdr(b)->len++, \
    dlb_vec_last_size(b, s))
// Pop & return pointer to last element, returns 0 if empty
#define dlb_vec_pop(b) \
    (dlb_vec_len(b) > 0 ? \
        dlb_vec_hdr(b)->len--, \
        (&(b)[dlb_vec_hdr(b)->len]) \
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

void *dlb_vec__grow(const void *buf, u32 len, u32 size);

#endif
//-- end of header -------------------------------------------------------------

//-- implementation ------------------------------------------------------------
#ifdef DLB_VECTOR_IMPLEMENTATION
#ifndef DLB_VECTOR_IMPLEMENTATION_DEF
#define DLB_VECTOR_IMPLEMENTATION_DEF

#include "dlb_types.h"
#define DLB_MEMORY_IMPLEMENTATION
#include "dlb_memory.h"

void *dlb_vec__grow(const void *buf, u32 len, u32 size) {
    dlb_vec__hdr *hdr = dlb_vec_hdr(buf);
    u32 cap = dlb_vec_cap(buf);
    DLB_ASSERT(cap <= (SIZE_MAX - 1) / 2);
    u32 new_cap = MAX(16, MAX(2 * cap, len));
    DLB_ASSERT(len <= new_cap);
    DLB_ASSERT(new_cap <= (SIZE_MAX - sizeof(dlb_vec__hdr))/size);
    u32 new_size = sizeof(dlb_vec__hdr) + new_cap * size;
    if (hdr) {
        DLB_ASSERT(!hdr->fixed);
        hdr = dlb_realloc(hdr, new_size);
        u32 old_size = sizeof(dlb_vec__hdr) + hdr->cap * size;
        dlb_memset((u8 *)hdr + old_size, 0, new_size - old_size);
    } else {
        hdr = dlb_calloc(1, new_size);
        hdr->len = 0;
    }
    hdr->cap = new_cap;
    u8 *new_buf = (u8 *)hdr + sizeof(dlb_vec__hdr);
    DLB_ASSERT(new_buf);
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
    for (u32 i = 0; i < dlb_vec_len(store); i++) {
        assert(store[i] == i);
    }
    dlb_vec_free(store);
}

#endif
#endif
