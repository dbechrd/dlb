//------------------------------------------------------------------------------
// Copyright 2019 Dan Bechard
//------------------------------------------------------------------------------

//-- header --------------------------------------------------------------------
#ifndef DLB_BITSET_H
#define DLB_BITSET_H

#include "dlb_types.h"
#include "dlb_vector.h"

// Store a bunch of flags as bits
typedef struct dlb_bitset {
    u32 *bitmaps;
} dlb_bitset;

static inline void dlb_bitset_reserve(dlb_bitset *bitset, u32 size)
{
    dlb_vec_reserve(bitset->bitmaps, size);
}

static inline void dlb_bitset_free(dlb_bitset *bitset)
{
    dlb_vec_free(bitset->bitmaps);
}

static inline void dlb_bitset_set(dlb_bitset *bitset, u32 index)
{
    u32 bitmap_idx = index >> 6;
    u8 bitmap_mask = 1 << (index & 0x1f);

    u32 bitmaps_len = dlb_vec_len(bitset->bitmaps);
    DLB_ASSERT(bitmap_idx < bitmaps_len);

    bitset->bitmaps[bitmap_idx] |= bitmap_mask;
}

static inline void dlb_bitset_unset(dlb_bitset *bitset, u32 index)
{
    u32 bitmap_idx = index >> 6;
    u8 bitmap_mask = 1 << (index & 0x1f);

    u32 bitmaps_len = dlb_vec_len(bitset->bitmaps);
    DLB_ASSERT(bitmap_idx < bitmaps_len);

    bitset->bitmaps[bitmap_idx] &= !bitmap_mask;
}

static inline u8 dlb_bitset_get(dlb_bitset *bitset, u32 index)
{
    u32 bitmap_idx = index >> 6;
    u8 bitmap_mask = 1 << (index & 0x1f);

    u32 bitmaps_len = dlb_vec_len(bitset->bitmaps);
    DLB_ASSERT(bitmap_idx < bitmaps_len);

    return bitset->bitmaps[bitmap_idx] & bitmap_mask;
}

#endif
//-- end of header -------------------------------------------------------------

//-- implementation ------------------------------------------------------------
#ifdef DLB_BITSET_IMPLEMENTATION
#ifndef DLB_BITSET_IMPLEMENTATION_DEF
#define DLB_BITSET_IMPLEMENTATION_DEF

// None for not, all inlined

#endif
#endif

//-- tests ---------------------------------------------------------------------
#ifdef DLB_BITSET_TEST
#ifndef DLB_BITSET_TEST_DEF
#define DLB_BITSET_TEST_DEF

static void *dlb_bitset_test()
{
    dlb_bitset bitset = { 0 };
    dlb_bitset_reserve(&bitset, 64);
    dlb_bitset_set(&bitset, 31);
    DLB_ASSERT(dlb_bitset_get(&bitset, 0) == 0);
    DLB_ASSERT(dlb_bitset_get(&bitset, 15) == 0);
    DLB_ASSERT(dlb_bitset_get(&bitset, 31) == 1);
    DLB_ASSERT(dlb_bitset_get(&bitset, 63) == 0);
    dlb_bitset_unset(&bitset, 31);
    DLB_ASSERT(dlb_bitset_get(&bitset, 31) == 0);
    dlb_bitset_free(&bitset);
}

#endif
#endif
