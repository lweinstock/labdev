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

#define debug_print(msg, ...) \
    do { if (PRINT_DEBUG) fprintf(stderr, "%s:%d [%s()]: " msg, \
        __FILE__, __LINE__, __func__, __VA_ARGS__); } while(0)
#endif