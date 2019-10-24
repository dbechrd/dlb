#ifndef DLB_INDEX_H
#define DLB_INDEX_H
//------------------------------------------------------------------------------
// Copyright 2019 Dan Bechard
//------------------------------------------------------------------------------

//-- header --------------------------------------------------------------------
#include "dlb_types.h"
#include "dlb_memory.h"
#include "dlb_hash.h"

typedef struct dlb_index {
    u32 buckets_count;
    u32 *buckets;
    u32 chains_count;
    u32 *chains;
    u32 grow_by;
} dlb_index;

void dlb_index_reserve(dlb_index *index, u32 buckets_count);

static inline void dlb_index_init(dlb_index *index, u32 buckets_count, u32 grow_by)
{
    // Elements must be at least large enough to hold a freelist
    dlb_index_reserve(index, buckets_count);
    index->grow_by = grow_by ? grow_by : index->buckets_count;
}

static inline void dlb_index_find(dlb_index *index, const char *key, u32 klen)
{

}

// WARNING: Direct access to dense array. Should only be used for fast iteration
//static inline void *dlb_index_at(dlb_index *index, u32 dense_index)
//{
//    DLB_ASSERT(index->dense_data);
//    DLB_ASSERT(dense_index < index->size);
//
//    void *ptr = (u8 *)index->dense_data + index->elem_size * dense_index;
//    return ptr;
//}

// Retrieve by id, safest access pattern (verfies dense_set match)
//static inline void *dlb_index_by_id(dlb_index *index, u32 id)
//{
//    DLB_ASSERT(index->sparse_set);
//    DLB_ASSERT(id < index->capacity);
//    u32 dense_index = index->sparse_set[id];
//
//    DLB_ASSERT(index->dense_set);
//    DLB_ASSERT(dense_index < index->size);
//    void *ptr = 0;
//    if (index->dense_set[dense_index] == id) {
//        ptr = dlb_index_at(pool, dense_index);
//    }
//    return ptr;
//}

static inline void *dlb_index_alloc(dlb_index *index, u32 id)
{
    if (index->size == index->capacity) {
        dlb_index_reserve(pool, index->capacity + index->grow_by);
    }
    DLB_ASSERT(index->size < index->capacity);

    void *ptr = (u8 *)index->dense_data + index->elem_size * index->size;
    index->sparse_set[id] = index->size;
    index->dense_set[index->size] = id;
    index->size++;
    return ptr;
}

static inline bool dlb_index_delete(dlb_index *index, u32 id)
{
    DLB_ASSERT(index->sparse_set);
    DLB_ASSERT(id < index->capacity);
    u32 dense_index = index->sparse_set[id];

    DLB_ASSERT(index->dense_set);
    DLB_ASSERT(dense_index < index->size);
    index->size--;

    bool found = false;
    if (index->dense_set[dense_index] == id) {
        found = true;
        void *ptr = dlb_index_at(pool, dense_index);
        void *last_ptr = dlb_index_at(pool, index->size);

        if (ptr != last_ptr) {
            // Compact (move last element to newly empty slot)
            u32 last_id = index->dense_set[index->size];
            index->sparse_set[last_id] = dense_index;
            index->dense_set[dense_index] = last_id;
            dlb_memcpy(ptr, last_ptr, index->elem_size);
        }

        // TODO(perf): None of this is actually necessary, just doing for debug
        // Clear deleted slot (invalidate sets)
        index->sparse_set[id] = 0xC0FFEE42;         // anything >= index->size
        index->dense_set[index->size] = 0xC0FFEE42;  // anything >= index->size
        dlb_memset(last_ptr, 0, index->elem_size);
    }
    return found;
}

static inline void dlb_index_free(dlb_index *index)
{
    dlb_free(index->sparse_set);
    dlb_free(index->dense_set);
    dlb_free(index->dense_data);
}

#endif
//-- end of header -------------------------------------------------------------

//-- implementation ------------------------------------------------------------
#ifdef DLB_INDEX_IMPLEMENTATION
#ifndef DLB_INDEX_IMPLEMENTATION_DEF
#define DLB_INDEX_IMPLEMENTATION_DEF

void dlb_index_reserve(dlb_index *index, u32 buckets_count)
{
    if (!buckets_count) {
        capacity = 16;
    }

    DLB_ASSERT(capacity > index->capacity);
    u32 old_capacity = index->capacity;
    index->capacity = capacity;

    if (index->sparse_set) {
        index->sparse_set = dlb_realloc(index->sparse_set, index->capacity * sizeof(*index->sparse_set));
        index->dense_set = dlb_realloc(index->dense_set, index->capacity * sizeof(*index->dense_set));
        index->dense_data = dlb_realloc(index->dense_data, index->capacity * index->elem_size);
    } else {
        index->sparse_set = dlb_calloc(index->capacity, sizeof(*index->sparse_set));
        index->dense_set = dlb_calloc(index->capacity, sizeof(*index->dense_set));
        index->dense_data = dlb_calloc(index->capacity, index->elem_size);
    }
}

#endif
#endif

//-- tests ---------------------------------------------------------------------
#ifdef DLB_INDEX_TEST
#ifndef DLB_INDEX_TEST_DEF
#define DLB_INDEX_TEST_DEF

static void dlb_index_test()
{
    struct some_data {
        u32 foo;
        u32 bar;
    };

    dlb_index pool = { 0 };
    dlb_index_init(&pool, 128, 0);
    dlb_index_free(&pool);
}

#endif
#endif
