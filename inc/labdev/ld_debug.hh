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

namespace labdev {

// Print message with additional information if LD_DEBUG is set
#define debug_print(msg, ...) \
    if (PRINT_DEBUG) ld_debug_print(stderr, __FILE__, __func__, msg, __VA_ARGS__)

// Print data string (max. 100 characters)
#define debug_print_string_data(data, msg, ...) \
    if (PRINT_DEBUG) { \
        ld_debug_print(stderr, __FILE__, __func__, msg, __VA_ARGS__); \
        ld_print_string_data(stderr, data); }

// Print data bytes (max. 60 characters)
#define debug_print_byte_data(data, len, msg, ...) \
    if (PRINT_DEBUG) { \
    ld_debug_print(stderr, __FILE__, __func__, msg, __VA_ARGS__); \
    ld_print_byte_data(stderr, data, len); }

void ld_debug_print(FILE* stream, const char* file, const char* function, 
    const char* msg, ...);
void ld_print_string_data(FILE* stream, std::string data);
void ld_print_byte_data(FILE* stream, const uint8_t* data, size_t len);

}

#endif