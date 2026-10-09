#include "screen_locker/lockscreen_args.h"

#include <string.h>
#include <unistd.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STBI_NO_HDR
#define STBI_NO_LINEAR
#include <stb/stb_image.h>

typedef enum
{
    LS_ARGV_TYPE = 1,

    LS_ARGV_COLOR = 2,

    LS_ARGV_PATH = 2,
} lockscreen_args_cli_args;

static int usage(char** argv)
{
    fprintf(stderr, "usage: %s [-i image.(png|jpg)] [-c RRGGBB]\n", argv[0]);
    return 1;
}

lockscreen_args parse_from_vargs(char** argv, int argc)
{
    lockscreen_args args = { .type = LOCKSCREEN_ARG_FLAT, .flat = { .color = 0 } };

    if (argc == 1)
    {
        return args;
    }

    if (argc != 3)
    {
        usage(argv);
    }

    const char* type = argv[LS_ARGV_TYPE];
    if (strcmp("-i", type) == 0)
    {
        const char* path = argv[LS_ARGV_PATH];
        if (access(path, R_OK) != 0)
        {
            usage(argv);
        }

        int w, h = 0;
        args.image.data   = (uint8_t*) stbi_load(path, &w, &h, NULL, 4);
        args.image.width  = w;
        args.image.height = h;
    }
    else if (strcmp("-c", type) == 0)
    {
        const char* color = argv[LS_ARGV_COLOR];
        args.flat.color   = strtoul(color, NULL, 16) & 0xFFFFFF;
    }

    return args;
}
