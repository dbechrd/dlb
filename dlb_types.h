#ifndef DLB_TYPES_H
#define DLB_TYPES_H

//------------------------------------------------------------------------------
// Basic type redefinitions
//------------------------------------------------------------------------------
#include <cstdint>
#include <climits>
//#include <stdbool.h>
//#include <assert.h>
//#include <float.h>
//#include <stddef.h>

typedef int8_t      s8;
typedef int16_t     s16;
typedef int32_t     s32;
typedef int64_t     s64;
typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;
typedef float       r32;
typedef double      r64;
typedef u32         b32;

typedef s8      int8;
typedef s16     int16;
typedef s32     int32;
typedef s64     int64;
typedef u8      uint8;
typedef u16     uint16;
typedef u32     uint32;
typedef u64     uint64;
typedef r32     real32;
typedef r64     real64;
//typedef u32     bool32;

// NOTE: internal and global are relative to translation unit
#if 0
#define local    static
#define internal static
#define global   static
#endif

// Enums generators
//
//   #define MENU_ITEMS(f) \
//       f(Main,  "Main Menu") \
//       f(Audio, "Audio")
//   DLB_ENUM_DECL(Menu, MENU_ITEMS);
//   DLB_ENUM_DEFS(Menu, MENU_ITEMS);
//   STRING(Menu::Main);
//
#define DLB_ENUM(e, ...) e,
#define DLB_ENUM_INT(e, val) e = val,
#define DLB_ENUM_STRINGIFY(e, ...) #e,
#define DLB_ENUM_DESCIFY(e, desc) desc,
#define DLB_ENUM_DECL(t, items_macro) \
    enum t { \
        items_macro(DLB_ENUM) \
    }; \
    extern const char *t##Str[]; \
    extern const char *t##Desc[];
#define DLB_ENUM_DEFS(t, items_macro) \
    const char *t##Str[] = { items_macro(DLB_ENUM_STRINGIFY) }; \
    const char *t##Desc[] = { items_macro(DLB_ENUM_DESCIFY) };
#define DLB_ENUM_STR(t, e) t##Str[(int)t::e]
#define DLB_ENUM_DESC(t, e) t##Desc[(int)t::e]

// Useful macros
#define UNUSED(x) ((void)(x))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define ABS(x) (((x) > 0) ? (x) : -(x))
#define CLAMP(x, min, max) (MAX((min), MIN((x), (max))))
#define LERP(a, b, alpha) ((a) + ((b) - (a)) * (alpha))

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))                      // NOTE: Old name is ARRAY_COUNT
#define FIELD_SIZEOF(type, field) (sizeof(((type *)0)->field))          // NOTE: Old name is SIZEOF_MEMBER
#define FIELD_SIZEOF_ARRAY(type, field) (sizeof(*(((type *)0)->field))) // NOTE: Old name is SIZEOF_MEMBER_ARRAY
#define OFFSETOF(type, field) ((size_t)&(((type *)0)->field))
#define STRING(s) #s
#define CSTR(s) (s), sizeof(s) - 1  // without terminator
#define CSTR0(s) (s), sizeof(s)     // with nil terminator
#define IFNULL(a, b) ((a) ? (a) : (b))

#define KB(bytes) (1024 * bytes)
#define MB(bytes) (1024 * KB(bytes))
#define GB(bytes) (1024 * MB(bytes))

// Note: Alignment must be power of 2
#define ALIGN_DOWN(n, a) ((n) & ~((a) - 1))
#define ALIGN_UP(n, a) ALIGN_DOWN((n) + (a) - 1, (a))
#define ALIGN_DOWN_PTR(p, a) ((void *)ALIGN_DOWN((uintptr_t)(p), (a)))
#define ALIGN_UP_PTR(p, a) ((void *)ALIGN_UP((uintptr_t)(p), (a)))

/*
// NOT SAFE TO USE IN WINDOWS 10, CAUSES ENTIRE OS TO HANG
#if defined(_MSC_VER)
    #define DLB_DEBUG_BREAK __debugbreak()
#elif defined(__GNUC__) || defined(__clang__)
    #define DLB_DEBUG_BREAK __builtin_trap()
#endif
*/

static inline u16 endian_swap_u16(u16 val)
{
    return (u16)(((val & 0xFF00) >> 8) |
                 ((val & 0x00FF) << 8));
}

static inline u32 endian_swap_u32(u32 val)
{
    return (u32)(((val & 0xFF000000) >> 24) |
                 ((val & 0x00FF0000) >>  8) |
                 ((val & 0x0000FF00) <<  8) |
                 ((val & 0x000000FF) << 24));
}

static inline void swap_r32(r32 *a, r32 *b)
{
    r32 t = *a;
    *a = *b;
    *b = t;
}

static inline void swap_int(int *a, int *b)
{
    int t = *a;
    *a = *b;
    *b = t;
}
#endif

//------------------------------------------------------------------------------
// HACK: I have no idea why this doesn't work when it's inside of the #include
// guards for unity builds... Defining this multiple times doesn't really hurt
// anything, so... fuck it.
//------------------------------------------------------------------------------
#define DLB_ASSERT_HANDLER(name) \
    void name(const char *expr, const char *filename, u32 line)
typedef DLB_ASSERT_HANDLER(dlb_assert_handler_def);
extern dlb_assert_handler_def *dlb_assert_handler;

#define DLB_ASSERT(expr) \
    do { \
        if (!(expr)) { \
            if (dlb_assert_handler) { \
                (*dlb_assert_handler)(#expr, __FILE__, __LINE__); \
            } \
            __debugbreak(); \
            exit(-1); \
        } \
    } while(0)
//------------------------------------------------------------------------------
