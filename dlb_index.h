#ifndef DLB_INDEX_H
#define DLB_INDEX_H
//------------------------------------------------------------------------------
// Copyright 2019 Dan Bechard
//------------------------------------------------------------------------------

//-- header --------------------------------------------------------------------
#include "dlb_types.h"
#include "dlb_memory.h"
#include "dlb_murmur3.h"

#define DLB_INDEX_EMPTY UINT32_MAX

typedef struct dlb_index {
    u32 buckets_count;
    u32 *buckets;
    u32 chains_count;
    u32 *chains;
    u32 grow_by;
} dlb_index;

static inline void dlb_index_init(dlb_index *store, u32 buckets, u32 chains)
{
    store->buckets_count = buckets;
    store->buckets = dlb_malloc(store->buckets_count * sizeof(*store->buckets));
    for (u32 i = 0; i < store->buckets_count; ++i) {
        store->buckets[i] = DLB_INDEX_EMPTY;
    }
    store->chains_count = chains;
    store->chains = dlb_malloc(store->chains_count * sizeof(*store->chains));
    for (u32 i = 0; i < store->chains_count; ++i) {
        store->chains[i] = DLB_INDEX_EMPTY;
    }
    store->grow_by = store->buckets_count;
}

static inline u32 dlb_index_first(dlb_index *store, u32 hash)
{
    u32 bucket = dlb_reduce(hash, store->buckets_count);
    DLB_ASSERT(bucket < store->buckets_count);  // NOTE: Remove once we're sure reduce is working
    u32 index = store->buckets[bucket];
    return index;
}

static inline u32 dlb_index_next(dlb_index *store, u32 index)
{
    DLB_ASSERT(index < store->chains_count);
    u32 next = store->chains[index];
    return next;
}

static inline void dlb_index_insert(dlb_index *store, u32 hash, u32 index)
{
    if (index >= store->chains_count) {
        DLB_ASSERT(!"TODO: Resize chains to index + 1");
    }
    u32 bucket = dlb_reduce(hash, store->buckets_count);
    DLB_ASSERT(bucket < store->buckets_count);  // NOTE: Remove once we're sure reduce is working
    DLB_ASSERT(index < store->chains_count);    // TODO: Resize chains
    store->chains[index] = store->buckets[bucket];
    store->buckets[bucket] = index;
}

static inline void dlb_index_delete(dlb_index *store, u32 hash, u32 index)
{
    u32 bucket = dlb_reduce(hash, store->buckets_count);
    DLB_ASSERT(bucket < store->buckets_count);  // NOTE: Remove once we're sure reduce is working
    if (store->buckets[bucket] == index) {
        store->buckets[bucket] = store->chains[index];
    } else {
        for (u32 i = store->buckets[bucket]; i != DLB_INDEX_EMPTY; i = store->chains[i]) {
            if (store->chains[i] == index) {
                store->chains[i] = store->chains[index];
                break;
            }
        }
    }
    DLB_ASSERT(index < store->chains_count);
    store->chains[index] = DLB_INDEX_EMPTY;
}

static inline void dlb_index_free(dlb_index *store)
{
    dlb_free(store->buckets);
    dlb_free(store->chains);
    dlb_memset(store, 0, sizeof(*store));
}

#endif
//-- end of header -------------------------------------------------------------

#ifdef __INTELLISENSE__
/* This makes MSVC intellisense work. */
#define DLB_INDEX_IMPLEMENTATION
#endif

//-- implementation ------------------------------------------------------------
#ifdef DLB_INDEX_IMPLEMENTATION
#ifndef DLB_INDEX_IMPL_INTERNAL
#define DLB_INDEX_IMPL_INTERNAL

// None for now, all inlined

#endif
#endif
//-- end of implementation -----------------------------------------------------

//-- tests ---------------------------------------------------------------------
#ifdef DLB_INDEX_TEST

static void dlb_index_test()
{
    dlb_index index = { 0 };
    dlb_index_init(&index, 128, 0);
    dlb_index_free(&index);
}

#endif
//-- end of tests --------------------------------------------------------------