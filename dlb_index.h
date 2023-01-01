#ifndef DLB_INDEX_H
#define DLB_INDEX_H
//------------------------------------------------------------------------------
// Copyright 2019 Dan Bechard
//------------------------------------------------------------------------------

//-- documentation -------------------------------------------------------------
// High-level flow of looking up a hash value:
//
//   hash = murmur3("uid_string")
//   bucket_idx = hash % buckets_count      // bucket_idx is effectively "chain[0]", i.e. first value is in `buckets`
//   chain_idx1 = buckets[bucket_idx]       // external dense array index -AND- "chain[1]" (or DLB_INDEX_EMPTY)
//   chain_idx2 = chains[chain_idx1]        // external dense array index -AND- "chain[2]" (or DLB_INDEX_EMPTY)
//   chain_idx3 = chains[chain_idx2]        // external dense array index -AND- "chain[3]" (or DLB_INDEX_EMPTY)
//   ... continue until DLB_INDEX_EMPTY
//
// The calling code is responsible for consuming the indices returned by this structure, using them to look up into
// whatever dense array structure is being used (usually a dlb_vector), and ensuring that the element at that index
// is actually a match (i.e. symbol/string comparison on name field, or some other effective GUID)
//
// When a match is encountered, the calling code can early exit. When there is no match, the calling code should
// continue calling dlb_index_next until either a match is found or DLB_INDEX_EMPTY is returned.
//
// Example usage:
#if 0
typedef struct EmployeeRecord {
    const char *name;
    int age;
} EmployeeRecord;

typedef struct EmployeeDatabase {
    EmployeeRecord *records;  // dynamic array
    dlb_index index;          // index into `records` array
} EmployeeDatabase;

EmployeeRecord *ta_employee_find(EmployeeDatabase *database, const char *name)
{
    // NOTE: Free free to remove sanity checks
    assert(database && name && database->records && database->index);

    // NOTE: If the length is known in advance, you can pass it in and remove this expensive strlen() call.
    u32 hash = dlb_murmur3(name, (u32)strlen(name));
    for (size_t i = dlb_index_first(database->index, hash); i != DLB_INDEX_EMPTY; i = dlb_index_next(database->index, i)) {
        EmployeeRecord *record = database->records[i];
        // NOTE: If your strings are interned or if you hash e.g. a `int employee_id;` field instead of a string, you
        // can remove this expensive strcmp() call.
        if (!strcmp(record->name, name)) {
            return record;
        }
    }
    return 0;
}
#endif

//-- header --------------------------------------------------------------------
#include "dlb_types.h"
#include "dlb_memory.h"
#include "dlb_murmur3.h"

#define DLB_INDEX_EMPTY SIZE_MAX

typedef struct dlb_index {
    // Buckets works as a normal hash table with 1 slot per hash code. That slot holds an integer value which is to be
    // used as the lookup index into an adjacent dense array (not part of this structure). That index is also used as
    // a lookup index into the sparse chains array below, which in turn contains the next index, forming a linked list
    // of spare indices until DLB_INDEX_EMPTY is encountered.
    size_t buckets_count;
    size_t *buckets;
    // This array is the external chaining mechanism for buckets. It contains linked lists of array indices into itself
    // which are also indices into the external dense array that this dlb_index structure is indexing.
    size_t chains_count;
    size_t *chains;
    size_t grow_by;
} dlb_index;

static inline void dlb_index_init(dlb_index *store, size_t buckets, size_t chains)
{
    store->buckets_count = buckets;
    store->buckets = (size_t *)dlb_malloc(store->buckets_count * sizeof(*store->buckets));
    for (size_t i = 0; i < store->buckets_count; ++i) {
        store->buckets[i] = DLB_INDEX_EMPTY;
    }
    store->chains_count = chains;
    store->chains = (size_t *)dlb_malloc(store->chains_count * sizeof(*store->chains));
    for (size_t i = 0; i < store->chains_count; ++i) {
        store->chains[i] = DLB_INDEX_EMPTY;
    }
    store->grow_by = store->buckets_count;
}

static inline size_t dlb_index_first(dlb_index *store, u32 hash)
{
    if (!store->buckets_count) {
        return DLB_INDEX_EMPTY;
    }
    size_t bucket = dlb_reduce(hash, store->buckets_count);
    DLB_ASSERT(bucket < store->buckets_count);  // NOTE: Remove once we're sure reduce is working
    size_t index = store->buckets[bucket];
    return index;
}

static inline size_t dlb_index_next(dlb_index *store, size_t index)
{
    DLB_ASSERT(index < store->chains_count);
    size_t next = store->chains[index];
    return next;
}

static inline void dlb_index_insert(dlb_index *store, u32 hash, size_t index)
{
    if (index >= store->chains_count) {
        if (!store->buckets_count) {
            DLB_ASSERT(!"index not initialized");
            //dlb_index_init(store, 128, 128);
        } else {
            DLB_ASSERT(!"TODO: Resize chains to index + 1");
        }
    }
    size_t bucket = dlb_reduce(hash, store->buckets_count);
    DLB_ASSERT(bucket < store->buckets_count);  // NOTE: Remove once we're sure reduce is working
    DLB_ASSERT(index < store->chains_count);    // TODO: Resize chains
    size_t cursor = store->buckets[bucket];
    if (cursor == DLB_INDEX_EMPTY) {
        store->buckets[bucket] = index;
    } else {
        while (store->chains[cursor] != DLB_INDEX_EMPTY) {
            cursor = store->chains[cursor];
        }
        store->chains[cursor] = index;
    }
}

static inline bool dlb_index_delete(dlb_index *store, u32 hash, size_t index)
{
    bool found = false;
    size_t bucket = dlb_reduce(hash, store->buckets_count);
    DLB_ASSERT(bucket < store->buckets_count);  // NOTE: Remove once we're sure reduce is working
    DLB_ASSERT(index < store->chains_count);
    if (store->buckets[bucket] == index) {
        store->buckets[bucket] = store->chains[index];
        found = true;
    } else {
        size_t chain_idx = store->buckets[bucket];
        while (chain_idx != DLB_INDEX_EMPTY) {
            if (store->chains[chain_idx] == index) {
                store->chains[chain_idx] = store->chains[index];
                found = true;
                break;
            }
            chain_idx = store->chains[chain_idx];
        }
    }
    store->chains[index] = DLB_INDEX_EMPTY;
    return found;
}

static inline void dlb_index_free(dlb_index *store)
{
    dlb_free(store->buckets);
    dlb_free(store->chains);
    dlb_memset(store, 0, sizeof(*store));
}

void dlb_index_test();

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

typedef struct {
    size_t index;
    const char *name;
    u32 name_hash;
    int age;
} EmployeeRecord;

typedef struct {
    EmployeeRecord *records;  // dynamic array
    dlb_index index;          // index into `records` array
} EmployeeDatabase;

void EmployeeDatabaseInit(EmployeeDatabase *database)
{
    dlb_index_init(&database->index, 8, 8);  // NOTE: Very small index to make forcing collisions easier
}

void EmployeeDatabaseFree(EmployeeDatabase *database)
{
    dlb_index_free(&database->index);
    dlb_vec_free(database->records);
}

EmployeeRecord *EmployeeFind(EmployeeDatabase *database, const char *name)
{
    // NOTE: If the length is known in advance, you can pass it in and remove this expensive strlen() call.
    u32 hash = dlb_murmur3(name, (u32)strlen(name));
    for (size_t i = dlb_index_first(&database->index, hash); i != DLB_INDEX_EMPTY; i = dlb_index_next(&database->index, i)) {
        EmployeeRecord *record = &database->records[i];
        // NOTE: If your strings are interned or if you hash e.g. a `int employee_id;` field instead of a string, you
        // can remove this expensive strcmp() call.
        if (!strcmp(record->name, name)) {
            return record;
        }
    }
    return 0;
}

void EmployeeHire(EmployeeDatabase *database, const char *name, int age)
{
    EmployeeRecord employee = { 0 };
    employee.index = dlb_vec_len(database->records);
    employee.name = name;
    employee.name_hash = dlb_murmur3(name, strlen(name));;
    employee.age = age;
    dlb_vec_push(database->records, employee);

    dlb_index_insert(&database->index, employee.name_hash, employee.index);
}

void EmployeeFire(EmployeeDatabase *database, const char *name)
{
    EmployeeRecord *employee = EmployeeFind(database, name);
    DLB_ASSERT(employee);
    dlb_index_delete(&database->index, employee->name_hash, employee->index);

    size_t record_count = dlb_vec_len(database->records);
    DLB_ASSERT(record_count);

    if (employee->index == record_count - 1) {
        dlb_vec_popz(database->records);
    } else {
        // Get last record in dense array, calculate hash and store old index
        size_t old_index = record_count - 1;
        size_t new_index = employee->index;
        u32 moving_hash = database->records[old_index].name_hash;

        // Move last record into newly empty slot, pop last record from dense array
        database->records[new_index] = database->records[old_index];
        database->records[new_index].index = new_index;
        dlb_vec_popz(database->records);

        // Update index for the moved record
        dlb_index_delete(&database->index, moving_hash, old_index);
        dlb_index_insert(&database->index, moving_hash, new_index);
    }
}

void dlb_index_test()
{
    EmployeeDatabase database = { 0 };
    EmployeeDatabaseInit(&database);

    EmployeeHire(&database, "Alice",  0);
    EmployeeHire(&database, "Bob",    1);
    EmployeeHire(&database, "Carol",  2);
    EmployeeHire(&database, "David",  3);
    EmployeeHire(&database, "Emily",  4);
    EmployeeHire(&database, "Frank",  5);
    EmployeeHire(&database, "George", 6);
    EmployeeHire(&database, "Harold", 7);

    EmployeeRecord *carol = EmployeeFind(&database, "Carol");
    EmployeeRecord *david = EmployeeFind(&database, "David");
    DLB_ASSERT(carol && carol->age == 2);
    DLB_ASSERT(david && david->age == 3);

    // Carol and David are chained, delete head of chain and ensure David gets moved correctly
    EmployeeFire(&database, "Carol");
    carol = EmployeeFind(&database, "Carol");
    david = EmployeeFind(&database, "David");
    DLB_ASSERT(!carol);
    DLB_ASSERT(david && david->age == 3);

    // You're all fired!
    while (dlb_vec_len(database.records)) {
        EmployeeFire(&database, database.records[0].name);
    }

    // Err.. please come back? I'll give you stock options! :)
    EmployeeHire(&database, "Alice",  30);
    EmployeeHire(&database, "Bob",    31);
    EmployeeHire(&database, "Carol",  32);
    EmployeeHire(&database, "David",  33);
    EmployeeHire(&database, "Emily",  34);
    EmployeeHire(&database, "Frank",  35);
    EmployeeHire(&database, "George", 36);
    EmployeeHire(&database, "Harold", 37);

    carol = EmployeeFind(&database, "Carol");
    david = EmployeeFind(&database, "David");
    DLB_ASSERT(carol && carol->age == 32);
    DLB_ASSERT(david && david->age == 33);

    // Carol and David are chained, delete tail of chain and ensure Carol does not get moved
    EmployeeFire(&database, "David");
    carol = EmployeeFind(&database, "Carol");
    david = EmployeeFind(&database, "David");
    DLB_ASSERT(carol && carol->age == 32);
    DLB_ASSERT(!david);

    EmployeeDatabaseFree(&database);
}

#endif
//-- end of tests --------------------------------------------------------------
