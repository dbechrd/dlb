#ifndef DLB_HASH_H
#define DLB_HASH_H
//------------------------------------------------------------------------------
// Copyright 2018 Dan Bechard
//------------------------------------------------------------------------------

//-- header --------------------------------------------------------------------
#include "dlb_types.h"
#include "dlb_murmur3.h"
#include <stdio.h>

typedef enum
{
    DLB_HASH_STRING,
    DLB_HASH_INT
} dlb_hash_type;

typedef struct
{
    const void *key;
    size_t klen;
    void *value;
} dlb_hash_entry;

typedef struct
{
    dlb_hash_type type;
    const char *name;
    size_t size;
    dlb_hash_entry *buckets;
    FILE *debug;
} dlb_hash;

#if DEBUG
// NOTE: For easier casting in debugger, make strings readable. Could use union instead, but wutevs for now.
typedef struct
{
    const char *key;
    size_t klen;
    void *value;
} dlb_hash_entry_str;

typedef struct
{
    dlb_hash_type type;
    const char *name;
    size_t size;
    dlb_hash_entry_str *buckets;
    FILE *debug;
} dlb_hash_str;
#endif

void dlb_hash_init(dlb_hash *table, dlb_hash_type type, const char *name,
    size_t size_pow2);
void dlb_hash_free(dlb_hash *table);
void dlb_hash_insert(dlb_hash *table, const void *key, size_t klen, void *value);
void *dlb_hash_search(dlb_hash *table, const void *key, size_t klen, int *found);
void dlb_hash_delete(dlb_hash *table, const void *key, size_t klen);

void dlb_hash_test();

#endif
//-- end of header -------------------------------------------------------------

#ifdef __INTELLISENSE__
/* This makes MSVC intellisense work. */
#define DLB_HASH_IMPLEMENTATION
#endif

//-- implementation ------------------------------------------------------------
#ifdef DLB_HASH_IMPLEMENTATION
#ifndef DLB_HASH_IMPL_INTERNAL
#define DLB_HASH_IMPL_INTERNAL

#include "dlb_memory.h"
#include <string.h>
#include <stdio.h>

const void *_DLB_HASH_FREED = (void *)INT32_MIN;

static size_t _dlb_hash_pow2(size_t x)
{
    return (x & (x - 1)) == 0;
}

void dlb_hash_init(dlb_hash *table, dlb_hash_type type, const char *name,
    size_t size_pow2)
{
    DLB_ASSERT(_dlb_hash_pow2(size_pow2));
    table->type = type;
    table->name = name;
    table->size = size_pow2;
    table->buckets = (dlb_hash_entry *)dlb_calloc(table->size,
        sizeof(table->buckets[0]));
#if _DEBUG
    if (table->debug) {
        fprintf(table->debug, "[hash][init] %s\n", table->name);
    }
#endif
}

void dlb_hash_free(dlb_hash *table)
{
#if _DEBUG
    if (table->debug) {
        fprintf(table->debug, "[hash][free] %s\n", table->name);
    }
#endif
    dlb_free(table->buckets);
}

static dlb_hash_entry *_dlb_hash_find(dlb_hash *table, const void *key, size_t klen, dlb_hash_entry **first_freed,
    u8 return_first)
{
    u32 hash = 0;
    switch (table->type) {
        case DLB_HASH_STRING:
            hash = dlb_murmur3(key, klen);
            break;
        case DLB_HASH_INT:
            hash = dlb_murmur3(&key, klen);
            break;
        default: DLB_ASSERT(0);
    }
    u32 index = hash & (table->size - 1);
    u32 i = 0;

#if _DEBUG
    if (table->debug) {
        fprintf(table->debug, "[hash][find] finding slot for %.*s, hash %u, starting at %u\n", (int)klen, (char *)key,
            hash, index);
    }
#endif

    dlb_hash_entry *entry = 0;
    for (;;) {
        entry = table->buckets + index;

        // Empty slot
        if (!entry->klen) {
            if (entry->key == _DLB_HASH_FREED) {
                if (return_first) {
                    break;
                } else if (first_freed && !*first_freed) {
                    *first_freed = entry;
                }
#if _DEBUG
                if (table->debug) {
                    fprintf(table->debug, "[hash][find] %u is empty (FREED)\n", index);
                }
#endif
            } else {
#if _DEBUG
                if (table->debug && !return_first) {
                    fprintf(table->debug, "[hash][find] %u is empty\n", index);
                }
#endif
                break;
            }
        }

        // Match
        int match = 0;
        if (entry->klen == klen) {
            switch (table->type) {
                case DLB_HASH_STRING: {
                    match = !strncmp((const char *)entry->key, (const char *)key, klen);
                    break;
                } case DLB_HASH_INT: {
                    match = entry->key == key;
                    break;
                } default: {
                    DLB_ASSERT(0);
                }
            }
        }
        if (match) {
            break;
        }

        // Next slot
        i++;
        index += (i * (i + 1)) >> 1;  // Same as (0.5f)i + (0.5f)i^2
        index &= (table->size - 1);

        // NOTE(perf): This check will check entire hash table if power-of-two
        //             size. This would be a good place to enforce max probes.
        // End of probe; not found
        if (i == table->size) {
            entry = 0;
            break;
        }
    }

#if _DEBUG
    if (table->debug) {
        fprintf(table->debug, "[hash][find] found slot at %u\n", index);
    }
#endif

    return entry;
}

void dlb_hash_insert(dlb_hash *table, const void *key, size_t klen, void *value)
{
    DLB_ASSERT(key);
    DLB_ASSERT(klen);

#if _DEBUG
    if (table->debug) {
        fprintf(table->debug, "[hash][insert] inserting value %p for key %.*s\n", value, (int)klen, (char *)key);
    }
#endif

    dlb_hash_entry *first_freed = 0;
    dlb_hash_entry *entry =
        _dlb_hash_find(table, key, klen, &first_freed, 1);

    if (entry) {
        entry->key = key;
        entry->klen = klen;
        entry->value = value;
    } else {
        // Out of memory
        // NOTE: In the power-of-two, triangle number case this won't happen
        //       until table is 100% full. We should realloc much sooner.
        DLB_ASSERT(!"DLB_HASH: Out of memory");  // TODO: Realloc hash table
    }
}
void *dlb_hash_search(dlb_hash *table, const void *key, size_t klen, int *found)
{
#if _DEBUG
    if (table->debug) {
        fprintf(table->debug, "[hash][search_start] searching for key %.*s\n", (int)klen, (char *)key);
    }
#endif

    //DLB_ASSERT(key);
    DLB_ASSERT(klen);
    void *value = NULL;
    int value_found = 0;

    dlb_hash_entry *first_freed = 0;
    dlb_hash_entry *entry = _dlb_hash_find(table, key, klen, &first_freed, 0);

    if (entry && entry->klen) {
        value = entry->value;
        value_found = 1;

        // Optimize by pushing entry into first free slot
        if (first_freed) {
            first_freed->key = entry->key;
            first_freed->klen = entry->klen;
            first_freed->value = entry->value;
            entry->key = _DLB_HASH_FREED;
            entry->klen = 0;
            entry->value = 0;
        }
    }

#if _DEBUG
    if (table->debug) {
        if (value_found) {
            fprintf(table->debug, "[hash][search_end] found value %p\n", value);
        } else {
            fprintf(table->debug, "[hash][search_end] not found\n");
        }
    }
#endif

    if (found) *found = value_found;
    return value;
}

// Possible improvements:
// https://en.wikipedia.org/wiki/Lazy_deletion
// https://attractivechaos.wordpress.com/2018/10/01/advanced-techniques-to-implement-fast-hash-tables/

void dlb_hash_delete(dlb_hash *table, const void *key, size_t klen)
{
    DLB_ASSERT(key);
    DLB_ASSERT(klen);

    dlb_hash_entry *entry = _dlb_hash_find(table, key, klen, 0, 0);

    if (entry && entry->klen) {
        entry->key = _DLB_HASH_FREED;
        entry->klen = 0;
        entry->value = 0;
    } else {
        DLB_ASSERT(0);  // Error: Tried to delete non-existent key
    }
}

#endif
#endif
//-- end of implementation -----------------------------------------------------

//-- tests ---------------------------------------------------------------------
#ifdef DLB_HASH_TEST

void dlb_hash_test() {
    dlb_hash table_ = { 0 };
    dlb_hash *table = &table_;
    dlb_hash_init(table, DLB_HASH_STRING, "hashtable", 4);
    const char key_1[] = "test key 1";
    const char key_2[] = "test key 2";
    const char key_3[] = "test key 3";
    const char val_1[] = "1st value";
    const char val_2[] = "2nd value";
    const char val_3[] = "3rd value";
    dlb_hash_insert(table, CSTR(key_1), (void *)val_1);
    dlb_hash_insert(table, CSTR(key_2), (void *)val_2);
    dlb_hash_insert(table, CSTR(key_3), (void *)val_3);

    const char *search_1;
    const char *search_2;
    const char *search_3;

    search_1 = dlb_hash_search(table, CSTR(key_1), 0);
    search_2 = dlb_hash_search(table, CSTR(key_2), 0);
    search_3 = dlb_hash_search(table, CSTR(key_3), 0);
    DLB_ASSERT(search_1 == val_1);
    DLB_ASSERT(search_2 == val_2);
    DLB_ASSERT(search_3 == val_3);

    dlb_hash_delete(table, CSTR(key_1));
    search_1 = dlb_hash_search(table, CSTR(key_1), 0);
    search_2 = dlb_hash_search(table, CSTR(key_2), 0);
    search_3 = dlb_hash_search(table, CSTR(key_3), 0);
    DLB_ASSERT(search_1 == 0);
    DLB_ASSERT(search_2 == val_2);
    DLB_ASSERT(search_3 == val_3);

    dlb_hash_delete(table, CSTR(key_2));
    search_1 = dlb_hash_search(table, CSTR(key_1), 0);
    search_2 = dlb_hash_search(table, CSTR(key_2), 0);
    search_3 = dlb_hash_search(table, CSTR(key_3), 0);
    DLB_ASSERT(search_1 == 0);
    DLB_ASSERT(search_2 == 0);
    DLB_ASSERT(search_3 == val_3);

    dlb_hash_delete(table, CSTR(key_3));
    search_1 = dlb_hash_search(table, CSTR(key_1), 0);
    search_2 = dlb_hash_search(table, CSTR(key_2), 0);
    search_3 = dlb_hash_search(table, CSTR(key_3), 0);
    DLB_ASSERT(search_1 == 0);
    DLB_ASSERT(search_2 == 0);
    DLB_ASSERT(search_3 == 0);

    dlb_hash_free(table);
}

#endif
//-- end of tests --------------------------------------------------------------
