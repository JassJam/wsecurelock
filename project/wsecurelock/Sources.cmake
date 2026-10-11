set(SOURCES
        src/main.c

        src/screen_locker/auth.c
        src/screen_locker/lockscreen_args.c
        src/screen_locker/screen_locker.c
        src/screen_locker/user.c
        src/screen_locker/draw.c

        src/screen_locker/registry/keyboard.h
        src/screen_locker/registry/keyboard.c
        src/screen_locker/registry/display.h
        src/screen_locker/registry/display.c
        src/screen_locker/registry/seat.h
        src/screen_locker/registry/seat.c
        src/screen_locker/registry/session.h
        src/screen_locker/registry/session.c
        src/screen_locker/registry/wayland.h
        src/screen_locker/registry/wayland.c
        src/screen_locker/registry/xdg-shell-client-protocol.h
        src/screen_locker/registry/xdg-shell-client-protocol.c

        src/screen_locker/utils/memory.c
        src/screen_locker/utils/memory.h
)

set(INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(HEADERS
        include/screen_locker/auth.h
        include/screen_locker/env.h
        include/screen_locker/lockscreen_args.h
        include/screen_locker/screen_locker.h
        include/screen_locker/user.h
        include/screen_locker/ui.h
        include/screen_locker/draw.h
        include/screen_locker/logger.h
)
