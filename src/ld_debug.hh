#ifndef LD_DEBUG_H
#define LD_DEBUG_H

/*
 *  LD_DEBUG can be used instead of PRINT_DEBUG, but some
 *  code editors complain if LD_DEBUG is not defined
 */

#ifdef LD_DEBUG
    #define PRINT_DEBUG 1
#else
    #define PRINT_DEBUG 0
#endif

// Print message with additional information if LD_DEBUG is set
#define debug_print(msg, ...) \
    do { if (PRINT_DEBUG) fprintf(stderr, "%s:%d [%s()]: " msg, \
        __FILE__, __LINE__, __func__, __VA_ARGS__); } while(0)

// Evaluate expression; if true -> return, if false -> abort()!
#define ld_assert(expr, msg, ...) \
    my_assert(expr, #expr, __FILE__, __func__, __LINE__, msg)

inline void my_assert(bool expr, const char* strexpr, const char* file,
const char* function, int line, const char* msg)
{
    if (!expr) {
        fprintf(stderr, "Assert failed:\t%s\n", msg);
        fprintf(stderr, "Expected:\t%s\n", strexpr);
        fprintf(stderr, "Source:\t %s (%s line %i)", function, file, line);
        abort();
    }
    return; 
}

#endif