//------------------------------------------------------------------------------
// Copyright 2019 Dan Bechard
//------------------------------------------------------------------------------

//-- header --------------------------------------------------------------------
#ifndef DLB_POOL_H
#define DLB_POOL_H

#include "dlb_types.h"
#include "dlb_memory.h"

// Fixed-size pool with intrusive free list
typedef struct dlb_pool {
    u32 capacity;
    u32 elem_size;
    u32 freelist;   // index of next free slot, == capacity when pool is full
    u32 size;
    void *data;
} dlb_pool;

void dlb_pool_reserve(dlb_pool *pool, u32 capacity);

static inline void dlb_pool_init(dlb_pool *pool, u32 elem_size, u32 capacity)
{
    // Elements must be at least large enough to hold a freelist
    DLB_ASSERT(elem_size >= sizeof(u32));
    pool->elem_size = elem_size;
    dlb_pool_reserve(pool, capacity);
}

static inline void *dlb_pool_at(dlb_pool *pool, u32 index)
{
    DLB_ASSERT(pool->data);
    DLB_ASSERT(index < pool->capacity);
    DLB_ASSERT(index < pool->size);
    void *ptr = (u8 *)pool->data + pool->elem_size * index;
    return ptr;
}

static inline void *dlb_pool_alloc(dlb_pool *pool)
{
    if (pool->freelist == pool->capacity) {
        dlb_pool_reserve(pool, pool->capacity << 1);
    }
    DLB_ASSERT(pool->freelist < pool->capacity);
    DLB_ASSERT(pool->size < pool->capacity);

    void *ptr = dlb_pool_at(pool, pool->freelist);
    pool->freelist = *(u32 *)ptr;
    pool->size++;
    return ptr;
}

static inline void dlb_pool_delete(dlb_pool *pool, u32 index)
{
    DLB_ASSERT(index < pool->capacity);
    DLB_ASSERT(pool->size);

    u32 last_idx = pool->size - 1;
    u32 *ptr = dlb_pool_at(pool, index);
    u32 *last = dlb_pool_at(pool, last_idx);

    if (last != ptr) {
        dlb_memcpy(ptr, last, pool->elem_size);
    }
    dlb_memset(last, 0, pool->elem_size);

    *last = pool->freelist;
    pool->freelist = last_idx;
    pool->size--;
}

static inline void dlb_pool_free(dlb_pool *pool)
{
    dlb_free(pool->data);
}

#endif
//-- end of header -------------------------------------------------------------

//-- implementation ------------------------------------------------------------
#ifdef DLB_POOL_IMPLEMENTATION
#ifndef DLB_POOL_IMPLEMENTATION_DEF
#define DLB_POOL_IMPLEMENTATION_DEF

void dlb_pool_reserve(dlb_pool *pool, u32 capacity)
{
    if (!capacity) {
        capacity = 16;
    }

    DLB_ASSERT(capacity > pool->capacity);
    u32 old_capacity = pool->capacity;
    pool->capacity = capacity;

    if (pool->data) {
        pool->data = dlb_realloc(pool->data, pool->capacity * pool->elem_size);
    } else {
        pool->data = dlb_calloc(pool->capacity, pool->elem_size);
    }

    // Initialize intrusive freelist
    for (u32 i = old_capacity; i < pool->capacity; ++i) {
        u32 *ptr = dlb_pool_at(pool, i);
        *ptr = i + 1;
    }
}

#endif
#endif

//-- tests ---------------------------------------------------------------------
#ifdef DLB_POOL_TEST
#ifndef DLB_POOL_TEST_DEF
#define DLB_POOL_TEST_DEF

static void *dlb_pool_test()
{
    struct some_data {
        u32 foo;
        u32 bar;
    };

    dlb_pool pool = { 0 };
    dlb_pool_init(&pool, sizeof(struct some_data));
    dlb_pool
    dlb_pool_free(&pool);
}

#endif
#endif
