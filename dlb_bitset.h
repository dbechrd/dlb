#ifndef DLB_BITSET_H
#define DLB_BITSET_H
//------------------------------------------------------------------------------
// Copyright 2019 Dan Bechard
//------------------------------------------------------------------------------

//-- header --------------------------------------------------------------------
#include "dlb_types.h"

// Store a bunch of flags as bits
typedef struct dlb_bitset {
    u32 size;
    u32 *bitmaps;
} dlb_bitset;

static inline void dlb_bitset_reserve(dlb_bitset *bitset, u32 size)
{
    bitset->size = size;
    bitset->bitmaps = dlb_calloc(size >> 6, sizeof(*bitset->bitmaps));
}

static inline void dlb_bitset_free(dlb_bitset *bitset)
{
    dlb_free(bitset->bitmaps);
    bitset->size = 0;
}

static inline void dlb_bitset_set(dlb_bitset *bitset, u32 index)
{
    u32 bitmap_idx = index >> 6;
    if (bitmap_idx >= bitset->size) {
        DLB_ASSERT(0);  // TODO: realloc
    }
    u8 bitmap_mask = 1 << (index & 0x1f);
    bitset->bitmaps[bitmap_idx] |= bitmap_mask;
}

static inline void dlb_bitset_unset(dlb_bitset *bitset, u32 index)
{
    u32 bitmap_idx = index >> 6;
    if (bitmap_idx < bitset->size) {
        u8 bitmap_mask = 1 << (index & 0x1f);
        bitset->bitmaps[bitmap_idx] &= !bitmap_mask;
    }
}

static inline u8 dlb_bitset_get(dlb_bitset *bitset, u32 index)
{
    u32 bitmap_idx = index >> 6;
    if (bitmap_idx < bitset->size) {
        u8 bitmap_mask = 1 << (index & 0x1f);
        return bitset->bitmaps[bitmap_idx] & bitmap_mask;
    } else {
        return 0;
    }
}

#endif
//-- end of header -------------------------------------------------------------

#ifdef __INTELLISENSE__
/* This makes MSVC intellisense work. */
#define DLB_BITSET_IMPLEMENTATION
#endif

//-- implementation ------------------------------------------------------------
#ifdef DLB_BITSET_IMPLEMENTATION
#ifndef DLB_BITSET_IMPL_INTERNAL
#define DLB_BITSET_IMPL_INTERNAL

// None for now, all inlined

#endif
#endif
//-- end of implementation -----------------------------------------------------

//-- tests ---------------------------------------------------------------------
#ifdef DLB_BITSET_TEST

static void dlb_bitset_test()
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
//-- end of tests --------------------------------------------------------------
