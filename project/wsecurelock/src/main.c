#include <stdio.h>
#include <stdlib.h>

#include "lockscreen_args.h"

int main(int argc, char** argv)
{
    lockscreen_args args = parse_from_vargs(argv, argc);
    return 0;
}
