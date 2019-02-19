//------------------------------------------------------------------------------
// Copyright 2018 Dan Bechard
//------------------------------------------------------------------------------

//-- header --------------------------------------------------------------------
#ifndef DLB_HASH_H
#define DLB_HASH_H

#include <stdio.h>
#include "dlb_types.h"

//-- Murmurhash3.h -------------------------------------------------------------
// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.

// Platform-specific functions and macros
#if defined(_MSC_VER) && (_MSC_VER < 1600)
    // Microsoft Visual Studio
    typedef unsigned char uint8_t;
    typedef unsigned int uint32_t;
    typedef unsigned __int64 uint64_t;
#else
    // Other compilers
    #include <stdint.h>
#endif

extern uint32_t MurmurHash3_seed;
void MurmurHash3_x86_32(const void *key, int len, void *out);
void MurmurHash3_x86_128(const void *key, int len, void *out);
void MurmurHash3_x64_128(const void *key, int len, void *out);

//-- dlb_hash.h ----------------------------------------------------------------
struct dlb_hash_entry
{
    const char *key;
    size_t klen;
    void *value;
};

struct dlb_hash
{
    const char *name;
    size_t size;
    struct dlb_hash_entry *buckets;
};

void dlb_hash_init(struct dlb_hash *table, const char *name,
                   size_t size_pow2);
void dlb_hash_free(struct dlb_hash *table);
void dlb_hash_insert(struct dlb_hash *table, const char *key, size_t klen,
                     void *value);
void *dlb_hash_search(struct dlb_hash *table, const char *key, size_t klen);
void dlb_hash_delete(struct dlb_hash *table, const char *key, size_t klen);

static inline u32 hash_string(const char *str, size_t len)
{
    u32 hash;
    MurmurHash3_x86_32(str, len, &hash);
    return hash;
}
#endif
//-- end of header -------------------------------------------------------------

//-- implementation ------------------------------------------------------------
#ifdef DLB_HASH_IMPLEMENTATION
#ifndef DLB_HASH_IMPLEMENTATION_DEF
#define DLB_HASH_IMPLEMENTATION_DEF

//-- MurmurHash3.c -------------------------------------------------------------
// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.

// Note - The x86 and x64 versions do _not_ produce the same results, as the
// algorithms are optimized for their respective platforms. You can still
// compile and run any of them on any platform, but your performance with the
// non-native version will be less than optimal.

// Platform-specific functions and macros
#if defined(_MSC_VER)
    // Microsoft Visual Studio
    #define FORCE_INLINE    __forceinline
    #include <stdlib.h>

    #define ROTL32(x,y)    _rotl(x,y)
    #define ROTL64(x,y)    _rotl64(x,y)
    #define BIG_CONSTANT(x) (x)
#else
    // Other compilers
    #define FORCE_INLINE inline __attribute__((always_inline))

    static FORCE_INLINE uint32_t rotl32(uint32_t x, int8_t r)
    {
        return (x << r) | (x >> (32 - r));
    }

    static FORCE_INLINE uint64_t rotl64(uint64_t x, int8_t r)
    {
        return (x << r) | (x >> (64 - r));
    }

    #define ROTL32(x,y)       rotl32(x,y)
    #define ROTL64(x,y)       rotl64(x,y)
    #define BIG_CONSTANT(x)   (x##LLU)
#endif

uint32_t MurmurHash3_seed = 0;

// Block read - if your platform needs to do endian-swapping or can only
// handle aligned reads, do the conversion here
FORCE_INLINE uint32_t getblock32(const uint32_t *p, int i)
{
    return p[i];
}

FORCE_INLINE uint64_t getblock64(const uint64_t *p, int i)
{
    return p[i];
}

// Finalization mix - force all bits of a hash block to avalanche
FORCE_INLINE uint32_t fmix32(uint32_t h)
{
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
}

FORCE_INLINE uint64_t fmix64(uint64_t k)
{
    k ^= k >> 33;
    k *= BIG_CONSTANT(0xff51afd7ed558ccd);
    k ^= k >> 33;
    k *= BIG_CONSTANT(0xc4ceb9fe1a85ec53);
    k ^= k >> 33;
    return k;
}

void MurmurHash3_x86_32(const void *key, int len, void *out)
{
    const uint8_t *data = (const uint8_t*)key;
    const int nblocks = len / 4;

    uint32_t h1 = MurmurHash3_seed;

    const uint32_t c1 = 0xcc9e2d51;
    const uint32_t c2 = 0x1b873593;

    //----------
    // body
    const uint32_t *blocks = (const uint32_t *)(data + nblocks*4);

    for(int i = -nblocks; i; i++)
    {
        uint32_t k1 = getblock32(blocks,i);

        k1 *= c1;
        k1 = ROTL32(k1,15);
        k1 *= c2;

        h1 ^= k1;
        h1 = ROTL32(h1,13);
        h1 = h1*5+0xe6546b64;
    }

    //----------
    // tail
    const uint8_t *tail = (const uint8_t*)(data + nblocks*4);
    uint32_t k1 = 0;

    switch(len & 3)
    {
    case 3: k1 ^= tail[2] << 16;
    case 2: k1 ^= tail[1] << 8;
    case 1: k1 ^= tail[0];
        k1 *= c1; k1 = ROTL32(k1,15); k1 *= c2; h1 ^= k1;
    };

    //----------
    // finalization
    h1 ^= len;
    h1 = fmix32(h1);
    *(uint32_t*)out = h1;
}

void MurmurHash3_x86_128(const void *key, int len, void *out)
{
    const uint8_t *data = (const uint8_t*)key;
    const int nblocks = len / 16;

    uint32_t h1 = MurmurHash3_seed;
    uint32_t h2 = MurmurHash3_seed;
    uint32_t h3 = MurmurHash3_seed;
    uint32_t h4 = MurmurHash3_seed;

    const uint32_t c1 = 0x239b961b;
    const uint32_t c2 = 0xab0e9789;
    const uint32_t c3 = 0x38b34ae5;
    const uint32_t c4 = 0xa1e38b93;

    //----------
    // body
    const uint32_t *blocks = (const uint32_t *)(data + nblocks*16);

    for(int i = -nblocks; i; i++)
    {
        uint32_t k1 = getblock32(blocks,i*4+0);
        uint32_t k2 = getblock32(blocks,i*4+1);
        uint32_t k3 = getblock32(blocks,i*4+2);
        uint32_t k4 = getblock32(blocks,i*4+3);

        k1 *= c1; k1  = ROTL32(k1,15); k1 *= c2; h1 ^= k1;
        h1 = ROTL32(h1,19); h1 += h2; h1 = h1*5+0x561ccd1b;

        k2 *= c2; k2  = ROTL32(k2,16); k2 *= c3; h2 ^= k2;
        h2 = ROTL32(h2,17); h2 += h3; h2 = h2*5+0x0bcaa747;

        k3 *= c3; k3  = ROTL32(k3,17); k3 *= c4; h3 ^= k3;
        h3 = ROTL32(h3,15); h3 += h4; h3 = h3*5+0x96cd1c35;

        k4 *= c4; k4  = ROTL32(k4,18); k4 *= c1; h4 ^= k4;
        h4 = ROTL32(h4,13); h4 += h1; h4 = h4*5+0x32ac3b17;
    }

    //----------
    // tail
    const uint8_t *tail = (const uint8_t*)(data + nblocks*16);

    uint32_t k1 = 0;
    uint32_t k2 = 0;
    uint32_t k3 = 0;
    uint32_t k4 = 0;

    switch(len & 15)
    {
    case 15: k4 ^= tail[14] << 16;
    case 14: k4 ^= tail[13] << 8;
    case 13: k4 ^= tail[12] << 0;
        k4 *= c4; k4  = ROTL32(k4,18); k4 *= c1; h4 ^= k4;

    case 12: k3 ^= tail[11] << 24;
    case 11: k3 ^= tail[10] << 16;
    case 10: k3 ^= tail[ 9] << 8;
    case  9: k3 ^= tail[ 8] << 0;
        k3 *= c3; k3  = ROTL32(k3,17); k3 *= c4; h3 ^= k3;

    case  8: k2 ^= tail[ 7] << 24;
    case  7: k2 ^= tail[ 6] << 16;
    case  6: k2 ^= tail[ 5] << 8;
    case  5: k2 ^= tail[ 4] << 0;
        k2 *= c2; k2  = ROTL32(k2,16); k2 *= c3; h2 ^= k2;

    case  4: k1 ^= tail[ 3] << 24;
    case  3: k1 ^= tail[ 2] << 16;
    case  2: k1 ^= tail[ 1] << 8;
    case  1: k1 ^= tail[ 0] << 0;
        k1 *= c1; k1  = ROTL32(k1,15); k1 *= c2; h1 ^= k1;
    };

    //----------
    // finalization
    h1 ^= len; h2 ^= len; h3 ^= len; h4 ^= len;

    h1 += h2; h1 += h3; h1 += h4;
    h2 += h1; h3 += h1; h4 += h1;

    h1 = fmix32(h1);
    h2 = fmix32(h2);
    h3 = fmix32(h3);
    h4 = fmix32(h4);

    h1 += h2; h1 += h3; h1 += h4;
    h2 += h1; h3 += h1; h4 += h1;

    ((uint32_t*)out)[0] = h1;
    ((uint32_t*)out)[1] = h2;
    ((uint32_t*)out)[2] = h3;
    ((uint32_t*)out)[3] = h4;
}

void MurmurHash3_x64_128(const void *key, int len, void *out)
{
    const uint8_t *data = (const uint8_t*)key;
    const int nblocks = len / 16;

    uint64_t h1 = MurmurHash3_seed;
    uint64_t h2 = MurmurHash3_seed;

    const uint64_t c1 = BIG_CONSTANT(0x87c37b91114253d5);
    const uint64_t c2 = BIG_CONSTANT(0x4cf5ad432745937f);

    //----------
    // body
    const uint64_t *blocks = (const uint64_t *)(data);

    for(int i = 0; i < nblocks; i++)
    {
        uint64_t k1 = getblock64(blocks,i*2+0);
        uint64_t k2 = getblock64(blocks,i*2+1);

        k1 *= c1; k1  = ROTL64(k1,31); k1 *= c2; h1 ^= k1;
        h1 = ROTL64(h1,27); h1 += h2; h1 = h1*5+0x52dce729;

        k2 *= c2; k2  = ROTL64(k2,33); k2 *= c1; h2 ^= k2;
        h2 = ROTL64(h2,31); h2 += h1; h2 = h2*5+0x38495ab5;
    }

    //----------
    // tail
    const uint8_t *tail = (const uint8_t*)(data + nblocks*16);

    uint64_t k1 = 0;
    uint64_t k2 = 0;

    switch(len & 15)
    {
    case 15: k2 ^= ((uint64_t)tail[14]) << 48;
    case 14: k2 ^= ((uint64_t)tail[13]) << 40;
    case 13: k2 ^= ((uint64_t)tail[12]) << 32;
    case 12: k2 ^= ((uint64_t)tail[11]) << 24;
    case 11: k2 ^= ((uint64_t)tail[10]) << 16;
    case 10: k2 ^= ((uint64_t)tail[ 9]) << 8;
    case  9: k2 ^= ((uint64_t)tail[ 8]) << 0;
        k2 *= c2; k2  = ROTL64(k2,33); k2 *= c1; h2 ^= k2;

    case  8: k1 ^= ((uint64_t)tail[ 7]) << 56;
    case  7: k1 ^= ((uint64_t)tail[ 6]) << 48;
    case  6: k1 ^= ((uint64_t)tail[ 5]) << 40;
    case  5: k1 ^= ((uint64_t)tail[ 4]) << 32;
    case  4: k1 ^= ((uint64_t)tail[ 3]) << 24;
    case  3: k1 ^= ((uint64_t)tail[ 2]) << 16;
    case  2: k1 ^= ((uint64_t)tail[ 1]) << 8;
    case  1: k1 ^= ((uint64_t)tail[ 0]) << 0;
        k1 *= c1; k1  = ROTL64(k1,31); k1 *= c2; h1 ^= k1;
    };

    //----------
    // finalization
    h1 ^= len; h2 ^= len;

    h1 += h2;
    h2 += h1;

    h1 = fmix64(h1);
    h2 = fmix64(h2);

    h1 += h2;
    h2 += h1;

    ((uint64_t*)out)[0] = h1;
    ((uint64_t*)out)[1] = h2;
}

//-- dlb_hash.c ----------------------------------------------------------------
#include <string.h>

const char *_DLB_HASH_FREED = "_dlb_hash_freed";

static u32 _dlb_hash_pow2(u32 x)
{
    return (x & (x - 1)) == 0;
}

void dlb_hash_init(struct dlb_hash *table, const char *name, size_t size_pow2)
{
    DLB_ASSERT(_dlb_hash_pow2(size_pow2));
    table->name = name;
	table->size = size_pow2;
    table->buckets = (struct dlb_hash_entry *)
		calloc(table->size, sizeof(table->buckets[0]));

#if DLB_HASH_DEBUG
    printf("[hash][init] %s\n", table->hnd.name);
#endif
}

void dlb_hash_free(struct dlb_hash *table)
{
#if DLB_HASH_DEBUG
    printf("[hash][free] %s\n", table->hnd.name);
#endif

    free(table->buckets);
}

static struct dlb_hash_entry *
_dlb_hash_find(struct dlb_hash *table, const char *key, u32 klen,
               struct dlb_hash_entry **first_freed, u8 return_first)
{
    u32 hash = hash_string(key, klen);
    u32 index = hash % table->size;
    u32 i = 0;

    struct dlb_hash_entry *entry = 0;
	for (;;)
	{
		entry = table->buckets + index;

		// Empty slot
		if (!entry->klen) {
            if (entry->key == _DLB_HASH_FREED) {
                if (return_first) {
                    break;
                } else if (first_freed && !*first_freed) {
                    *first_freed = entry;
                }
            } else {
                break;
            }
		}

		// Match
        if (entry->klen == klen && !strncmp(entry->key, key, klen)) {
			break;
		}

		// Next slot
        i++;
        index += i * (i + 1) / 2;  // Same as (0.5f)i + (0.5f)i^2
        index %= table->size;

        // End of probe; not found
		if (i == table->size) {
			entry = 0;
			break;
		}
	}

    return entry;
}

void dlb_hash_insert(struct dlb_hash *table, const char *key, size_t klen,
                     void *value)
{
    DLB_ASSERT(key);
    DLB_ASSERT(klen);

    struct dlb_hash_entry *first_freed = 0;
    struct dlb_hash_entry *entry =
        _dlb_hash_find(table, key, klen, &first_freed, 1);

    if (entry) {
        entry->key = key;
        entry->klen = klen;
        entry->value = value;
    } else {
        // Out of memory
        // NOTE: In the power-of-two, triangle number case this won't happen
        //       until table is 100% full. We should realloc much sooner.
        DLB_ASSERT(0);  // TODO: Realloc hash table
    }
}
void *dlb_hash_search(struct dlb_hash *table, const char *key, size_t klen)
{
    DLB_ASSERT(key);
    DLB_ASSERT(klen);
    void *value = NULL;

    struct dlb_hash_entry *first_freed = 0;
    struct dlb_hash_entry *entry =
        _dlb_hash_find(table, key, klen, &first_freed, 0);

    if (entry && entry->klen) {
        value = entry->value;

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

    return value;
}

// Possible improvements:
// https://en.wikipedia.org/wiki/Lazy_deletion
// https://attractivechaos.wordpress.com/2018/10/01/advanced-techniques-to-implement-fast-hash-tables/

void dlb_hash_delete(struct dlb_hash *table, const char *key, size_t klen)
{
    DLB_ASSERT(key);
    DLB_ASSERT(klen);

    struct dlb_hash_entry *entry = _dlb_hash_find(table, key, klen, 0, 0);

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

//-- tests ---------------------------------------------------------------------
#ifdef DLB_HASH_TEST
#ifndef DLB_HASH_TEST_DEF
#define DLB_HASH_TEST_DEF

static void dlb_hash_test() {
    struct dlb_hash table_;
    struct dlb_hash *table = &table_;
    dlb_hash_init(table, "hashtable", 4);
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

    search_1 = dlb_hash_search(table, CSTR(key_1));
    search_2 = dlb_hash_search(table, CSTR(key_2));
    search_3 = dlb_hash_search(table, CSTR(key_3));
    DLB_ASSERT(search_1 == val_1);
    DLB_ASSERT(search_2 == val_2);
    DLB_ASSERT(search_3 == val_3);

    dlb_hash_delete(table, CSTR(key_1));
    search_1 = dlb_hash_search(table, CSTR(key_1));
    search_2 = dlb_hash_search(table, CSTR(key_2));
    search_3 = dlb_hash_search(table, CSTR(key_3));
    DLB_ASSERT(search_1 == 0);
    DLB_ASSERT(search_2 == val_2);
    DLB_ASSERT(search_3 == val_3);

    dlb_hash_delete(table, CSTR(key_2));
    search_1 = dlb_hash_search(table, CSTR(key_1));
    search_2 = dlb_hash_search(table, CSTR(key_2));
    search_3 = dlb_hash_search(table, CSTR(key_3));
    DLB_ASSERT(search_1 == 0);
    DLB_ASSERT(search_2 == 0);
    DLB_ASSERT(search_3 == val_3);

    dlb_hash_delete(table, CSTR(key_3));
    search_1 = dlb_hash_search(table, CSTR(key_1));
    search_2 = dlb_hash_search(table, CSTR(key_2));
    search_3 = dlb_hash_search(table, CSTR(key_3));
    DLB_ASSERT(search_1 == 0);
    DLB_ASSERT(search_2 == 0);
    DLB_ASSERT(search_3 == 0);

    dlb_hash_free(table);
}
#endif
#endif