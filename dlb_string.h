#ifndef DLB_STRING_H
#define DLB_STRING_H

#include "dlb_types.h"

// str: nil-terminated string
// returns: length of string, not including the nil terminator
static inline u32 dlb_str_len(const char *str)
{
    u32 len = 0;
    while (*str++) {
        len++;
    }
    return len;
}

// str: nil-terminated string or NULL
// prefix: nil-terminated string or NULL
// returns: 1 if str starts with entire prefix, 0 otherwise
static inline int dlb_str_startswith(const char *str, const char *prefix)
{
    if (!str || !prefix) {
        return 0;
    }

    const char *str_ptr = str;
    const char *prefix_ptr = prefix;
    while (*str_ptr && *prefix_ptr && *str_ptr == *prefix_ptr) {
        str_ptr++;
        prefix_ptr++;
    }
    return *prefix_ptr == 0;
}

// str: nil-terminated string
// c: character to replace
// new_c: character to replace with
static inline void dlb_str_replace_char(char *str, char c, char new_c)
{
    char *str_ptr = str;
    while (*str_ptr) {
        if (*str_ptr == c) {
            *str_ptr = new_c;
        }
        str_ptr++;
    }
}

#if 0
#define STR(str) (const struct dlb_string) { str, sizeof(str) }
#define STRL(str, len) (const struct dlb_string) { str, len }

struct dlb_string
{
    const char *s;
    u32 len;
};

// NOTE: Don't remember what these do.. not well tested. Test before using.
static inline char *dlb_strsep_c(char **stringp, const char delim)
{
    char *start = *stringp;

    while (**stringp) {
        if (**stringp == delim) {
            **stringp = '\0';
            (*stringp)++;
            break;
        }
        (*stringp)++;
    }

    return start;
}

static inline char *dlb_strsep(char **stringp, const char *delims)
{
    char *start = *stringp;
    const char *delim;

    while (**stringp) {
        delim = delims;
        while (delim) {
            if (**stringp == *delim) {
                **stringp = '\0';
                (*stringp)++;
                return start;
            }
            delim++;
        }
        (*stringp)++;
    }

    return start;
}

// str: nil-terminated string containing only digits 0-9
// returns: number contained in string as long
static inline long dlb_atol(const char *str)
{
    if (!str) return 0;

    long val = 0;
    while(*str) {
        val = val*10 + (*str++ - '0');
    }

    return val;
}
#endif

#if 0
struct dlb_string
{
    u32 buffer_len;
    const char *str;
};

struct dlb_string *dlb_string_alloc(const char *str)
{
    u32 buffer_len = strlen(str);
    struct dlb_string *s = calloc(1, sizeof(struct dlb_string) + strlen(str) +
                                  1);
    s->buffer_len = buffer_len;
    s->str = s + 1;
    return s;
}

typedef struct Intern {
    size_t len;
    const char *str;
} Intern;

Arena str_arena;
Intern *interns;

const char *str_intern_range(const char *start, const char *end)
{
    size_t len = end - start;
    for (Intern *it = interns; it != buf_end(interns); it++) {
        if (it->len == len && strncmp(it->str, start, len) == 0) {
            return it->str;
        }
    }
    char *str = arena_alloc(&str_arena, len + 1);
    memcpy(str, start, len);
    str[len] = 0;
    buf_push(interns, (Intern) { len, str });
    return str;
}

const char *str_intern(const char *str)
{
    return str_intern_range(str, str + strlen(str));
}
#endif

#endif
