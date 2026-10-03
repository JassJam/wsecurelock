#pragma once

typedef enum
{
    LOCKSCREEN_ARG_FLAT,
    LOCKSCREEN_ARG_IMG
} lockscreen_args_type;

typedef struct
{
    const char* image_source;
    int width;
    int height;
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

lockscreen_args parse_from_vargs(const char** argv, int argc);
