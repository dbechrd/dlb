#ifndef DLB_ARENA_H
#define DLB_ARENA_H
//------------------------------------------------------------------------------
// Copyright 2018 Dan Bechard
//------------------------------------------------------------------------------

//-- header --------------------------------------------------------------------
#include "dlb_memory.h"
#include "dlb_vector.h"

typedef struct dlb_arena {
    char *ptr;      // Next free
    char *end;      // End of current block
    char **blocks;  // Linked list of allocated blocks
} dlb_arena;

#define DLB_ARENA_ALIGNMENT 8
#define DLB_ARENA_BLOCK_SIZE 1024

void dlb_arena__grow(dlb_arena *arena, size_t min_size);
void *dlb_arena_alloc(dlb_arena *arena, size_t size);
void dlb_arena_free(dlb_arena *arena);

#endif
//-- end of header -------------------------------------------------------------

#ifdef __INTELLISENSE__
/* This makes MSVC intellisense work. */
#define DLB_ARENA_IMPLEMENTATION
#endif

//-- implementation ------------------------------------------------------------
#ifdef DLB_ARENA_IMPLEMENTATION
#ifndef DLB_ARENA_IMPL_INTERNAL
#define DLB_ARENA_IMPL_INTERNAL

void dlb_arena__grow(dlb_arena *arena, size_t min_size) {
    size_t size = ALIGN_UP(MAX(DLB_ARENA_BLOCK_SIZE, min_size),
                           DLB_ARENA_ALIGNMENT);
    arena->ptr = dlb_malloc(size);
    arena->end = arena->ptr + size;
    dlb_vec_push(arena->blocks, arena->ptr);
}

void *dlb_arena_alloc(dlb_arena *arena, size_t size) {
    // If current block isn't big enough, stop using it and allocate a new one
    if (size > (size_t)(arena->end - arena->ptr)) {
        dlb_arena__grow(arena, size);
        assert(size <= (size_t)(arena->end - arena->ptr));
    }
    void *ptr = arena->ptr;
    arena->ptr = ALIGN_UP_PTR(arena->ptr + size, DLB_ARENA_ALIGNMENT);
    assert(arena->ptr <= arena->end);
    assert(ptr == ALIGN_DOWN_PTR(ptr, DLB_ARENA_ALIGNMENT));
    return ptr;
}

void dlb_arena_free(dlb_arena *arena) {
    for (char **it = arena->blocks; it != dlb_vec_end(arena->blocks); it++) {
        free(*it);
    }
}

#endif
#endif
//-- end of implementation -----------------------------------------------------
