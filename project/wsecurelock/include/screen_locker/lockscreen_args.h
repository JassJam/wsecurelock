#pragma once

#include <stdint.h>

typedef enum
{
    LOCKSCREEN_ARG_FLAT,
    LOCKSCREEN_ARG_IMG
} lockscreen_args_type;

typedef struct
{
    uint8_t* data;
    uint32_t width;
    uint32_t height;
} lockscreen_args_image;

typedef struct
{
    unsigned int color;
} lockscreen_args_flat;

typedef struct lockscreen_args
{
    lockscreen_args_type type;
    union
    {
        lockscreen_args_flat  flat;
        lockscreen_args_image image;
    };
} lockscreen_args;

lockscreen_args parse_from_vargs(char** argv, int argc);
