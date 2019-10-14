//------------------------------------------------------------------------------
// Copyright 2019 Dan Bechard
//------------------------------------------------------------------------------

//-- header --------------------------------------------------------------------
#ifndef DLB_POOL_H
#define DLB_POOL_H

#include "dlb_types.h"
#include "dlb_memory.h"

typedef struct dlb_id {
    u32 index;
    u32 generation;
} dlb_id;

// Fixed-size pool with intrusive free list
typedef struct dlb_pool {
    u32 capacity;       // number of slots allocated
    u32 size;           // number of slots occupied
    u32 elem_size;      // size of each element in dense set
    u32 free_head;      // first free sparse slot, == capacity when pool is full
    dlb_id *sparse_set; // indices into dense (do not reorder, do not shrink)
    void *dense_set;    // tightly packed data for fast iteration
} dlb_pool;

void dlb_pool_reserve(dlb_pool *pool, u32 capacity);

static inline void dlb_pool_init(dlb_pool *pool, u32 elem_size, u32 capacity)
{
    // Elements must be at least large enough to hold a freelist
    DLB_ASSERT(elem_size >= sizeof(u32));
    pool->elem_size = elem_size;
    dlb_pool_reserve(pool, capacity);
}

static inline void *dlb_pool_at(dlb_pool *pool, u32 dense_index)
{
    DLB_ASSERT(pool->size);
    DLB_ASSERT(pool->dense_set);

    void *ptr = (u8 *)pool->dense_set + pool->elem_size * dense_index;
    return ptr;
}

static inline void *dlb_pool_retrieve(dlb_pool *pool, dlb_id id)
{
    DLB_ASSERT(id.index < pool->capacity);
    DLB_ASSERT(pool->size);
    DLB_ASSERT(pool->dense_set);

    dlb_id dense_id = pool->sparse_set[id.index];
    DLB_ASSERT(dense_id.generation == id.generation);
    void *ptr = dlb_pool_at(pool, dense_id.index);
    return ptr;
}

static inline dlb_id dlb_pool_alloc(dlb_pool *pool, void **ptr)
{
    if (pool->free_head == pool->capacity) {
        dlb_pool_reserve(pool, pool->capacity << 1);
    }
    DLB_ASSERT(pool->free_head < pool->capacity);
    DLB_ASSERT(pool->size < pool->capacity);

    dlb_id id = { 0 };
    id.index = pool->free_head;
    id.generation = pool->sparse_set[pool->free_head].generation;
    pool->free_head = pool->sparse_set[pool->free_head].index;
    pool->sparse_set[id.index].index = pool->size;

    if (ptr) *ptr = dlb_pool_at(pool, pool->size);
    pool->size++;
    return id;
}

static inline void dlb_pool_delete(dlb_pool *pool, dlb_id id)
{
    DLB_ASSERT(id.index < pool->capacity);
    DLB_ASSERT(pool->size);
    DLB_ASSERT(pool->dense_set);

    dlb_id *dense_id = &pool->sparse_set[id.index];
    DLB_ASSERT(dense_id->generation == id.generation);

    u32 last_idx = pool->size - 1;
    void *ptr = dlb_pool_at(pool, dense_id->index);
    void *last = dlb_pool_at(pool, last_idx);

    if (last != ptr) {
        dlb_memcpy(ptr, last, pool->elem_size);

        // TODO(perf): Faster way to update sparse set when filling holes? The
        // only faster way I can think of is to add u32 overhead to dlb_id and
        // store "prev", or store relative offsets. The former seems like a
        // waste of space and the latter seems overly complicated.
        for (u32 i = 0; i < pool->capacity; ++i) {
            if (pool->sparse_set[i].index == last_idx) {
                pool->sparse_set[i].index = dense_id->index;
                break;
            }
        }
    }
    dlb_memset(last, 0, pool->elem_size);

    // TODO: If generation runs out we can move deleted to end of free list
    // instead of beginning to make reuse take much longer.
    dense_id->index = pool->free_head;
    dense_id->generation++;
    pool->free_head = id.index;
    pool->size--;
}

static inline void dlb_pool_free(dlb_pool *pool)
{
    dlb_free(pool->dense_set);
    dlb_free(pool->sparse_set);
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

    if (pool->dense_set) {
        pool->dense_set = dlb_realloc(pool->dense_set, pool->capacity * pool->elem_size);
        pool->sparse_set = dlb_realloc(pool->sparse_set, pool->capacity * sizeof(*pool->sparse_set));
    } else {
        pool->dense_set = dlb_calloc(pool->capacity, pool->elem_size);
        pool->sparse_set = dlb_calloc(pool->capacity, sizeof(*pool->sparse_set));
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
