#include "screen_locker/auth.h"
#include "screen_locker/lockscreen_args.h"
#include "screen_locker/screen_locker.h"

#include <stdio.h>

int main(int argc, char** argv)
{
    const lockscreen_args args = parse_from_vargs(argv, argc);

    screen_locker sl;
    screen_locker_init(&args, &sl);
    while (screen_locker_dispatch(&sl))
    {
    }
    screen_locker_destroy(&sl);

    return 0;
}
