#pragma once

#include <stdarg.h>
#include <stdio.h>
#include <assert.h>

static void sl_print(FILE* stream, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    fprintf(stream, "[wsecurelock] - ");
    vfprintf(stream, format, args);
    putc('\n', stream);
    va_end(args);
}

#define sl_assert(cond, ...)                                                                       \
    do                                                                                             \
    {                                                                                              \
        if (!(cond))                                                                               \
        {                                                                                          \
            sl_print(stderr, __VA_ARGS__);                                                         \
            assert(false);                                                                         \
        }                                                                                          \
    } while (false)

#define sl_print_debug(...) sl_print(stdout, __VA_ARGS__)
#define sl_print_error(...) sl_print(stderr, __VA_ARGS__)
