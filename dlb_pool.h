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
    u32 capacity;       // number of slots allocated
    u32 size;           // number of slots occupied
    u32 elem_size;      // size of each element in dense set
    u32 *sparse_set;    // uid -> dense index (do not reorder, do not shrink)
    u32 *dense_set;     // dense set of uids
    void *dense_data;   // tightly packed data for fast iteration
} dlb_pool;

void dlb_pool_reserve(dlb_pool *pool, u32 capacity);

static inline void dlb_pool_init(dlb_pool *pool, u32 elem_size, u32 capacity)
{
    // Elements must be at least large enough to hold a freelist
    pool->elem_size = elem_size;
    dlb_pool_reserve(pool, capacity);
}

// WARNING: Direct access to dense array. Should only be used for fast iteration
static inline void *dlb_pool_at(dlb_pool *pool, u32 dense_index)
{
    DLB_ASSERT(pool->dense_data);
    DLB_ASSERT(dense_index < pool->size);

    void *ptr = (u8 *)pool->dense_data + pool->elem_size * dense_index;
    return ptr;
}

// Retrieve by id, safest access pattern (verfies dense_set match)
static inline void *dlb_pool_by_id(dlb_pool *pool, u32 id)
{
    DLB_ASSERT(pool->sparse_set);
    DLB_ASSERT(id < pool->capacity);
    u32 dense_index = pool->sparse_set[id];

    DLB_ASSERT(pool->dense_set);
    DLB_ASSERT(dense_index < pool->size);
    void *ptr = 0;
    if (pool->dense_set[dense_index] == id) {
        ptr = dlb_pool_at(pool, dense_index);
    }
    return ptr;
}

static inline void *dlb_pool_alloc(dlb_pool *pool, u32 id)
{
    if (pool->size == pool->capacity) {
        dlb_pool_reserve(pool, pool->capacity << 1);
    }
    DLB_ASSERT(pool->size < pool->capacity);

    void *ptr = dlb_pool_at(pool, pool->size);
    pool->sparse_set[id] = pool->size;
    pool->dense_set[pool->size] = id;
    pool->size++;
    return ptr;
}

static inline bool dlb_pool_delete(dlb_pool *pool, u32 id)
{
    DLB_ASSERT(pool->sparse_set);
    DLB_ASSERT(id < pool->capacity);
    u32 dense_index = pool->sparse_set[id];

    DLB_ASSERT(pool->dense_set);
    DLB_ASSERT(dense_index < pool->size);
    pool->size--;

    bool found = false;
    if (pool->dense_set[dense_index] == id) {
        found = true;
        void *ptr = dlb_pool_at(pool, dense_index);
        void *last_ptr = dlb_pool_at(pool, pool->size);

        if (ptr != last_ptr) {
            // Compact (move last element to newly empty slot)
            u32 last_id = pool->dense_set[pool->size];
            pool->sparse_set[last_id] = dense_index;
            pool->dense_set[dense_index] = last_id;
            dlb_memcpy(ptr, last_ptr, pool->elem_size);
        }

        // TODO(perf): None of this is actually necessary, just doing for debug
        // Clear deleted slot (invalidate sets)
        pool->sparse_set[id] = 0xC0FFEE42;         // anything >= pool->size
        pool->dense_set[pool->size] = 0xC0FFEE42;  // anything >= pool->size
        dlb_memset(last_ptr, 0, pool->elem_size);
    }
    return found;
}

static inline void dlb_pool_free(dlb_pool *pool)
{
    dlb_free(pool->sparse_set);
    dlb_free(pool->dense_set);
    dlb_free(pool->dense_data);
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

    if (pool->sparse_set) {
        pool->sparse_set = dlb_realloc(pool->sparse_set, pool->capacity * sizeof(*pool->sparse_set));
        pool->dense_set = dlb_realloc(pool->dense_set, pool->capacity * sizeof(*pool->dense_set));
        pool->dense_data = dlb_realloc(pool->dense_data, pool->capacity * pool->elem_size);
    } else {
        pool->sparse_set = dlb_calloc(pool->capacity, sizeof(*pool->sparse_set));
        pool->dense_set = dlb_calloc(pool->capacity, sizeof(*pool->dense_set));
        pool->dense_data = dlb_calloc(pool->capacity, pool->elem_size);
    }

    // Initialize intrusive freelist
    for (u32 i = old_capacity; i < pool->capacity; ++i) {
        dlb_id *id = pool->sparse_set[i];
        id->index = i + 1;
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
    //dlb_pool
    dlb_pool_free(&pool);
}

#endif
#endif
