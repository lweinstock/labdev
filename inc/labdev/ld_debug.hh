#ifndef LD_DEBUG_H
#define LD_DEBUG_H

#include <unistd.h>
#include <stdio.h>
#include <string>
#include <stdarg.h>

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
    if (PRINT_DEBUG) my_debug_print(stderr, __FILE__, __LINE__, __func__, msg, __VA_ARGS__)

// Print data string (max. 100 characters)
#define debug_print_string_data(data, msg, ...) \
    if (PRINT_DEBUG) { \
        my_debug_print(stderr, __FILE__, __LINE__, __func__, msg, __VA_ARGS__); \
        my_print_string_data(stderr, data); }

// Print data bytes (max. 60 characters)
#define debug_print_byte_data(data, len, msg, ...) \
    if (PRINT_DEBUG) { \
    my_debug_print(stderr, __FILE__, __LINE__, __func__, msg, __VA_ARGS__); \
    my_print_byte_data(stderr, data, len); }

void my_debug_print(FILE* stream, const char* file, int line, 
    const char* function, const char* msg, ...);
void my_print_string_data(FILE* stream, std::string data);
void my_print_byte_data(FILE* stream, const uint8_t* data, size_t len);

// Evaluate expression; if true -> return, if false -> abort()!
#define ld_assert(expr, msg, ...) \
    my_assert(expr, #expr, __FILE__, __func__, __LINE__, msg)

inline void my_assert(bool expr, const char* strexpr, const char* file,
    const char* function, int line, const char* msg);

#endif