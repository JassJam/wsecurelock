#pragma once

#include "lockscreen_args.h"
#include "password.h"
#include "ui.h"

#include <stdbool.h>

typedef struct screen_locker
{
    char             user[64];
    password_t       pw;
    screen_locker_ui ui;

    struct wl_display*  display;
    struct xkb_context* kb_context;

    struct wl_compositor* compositor;
    struct wl_shm*        shared_memory;
    struct wl_seat*       seat;
    struct wl_keyboard*   keyboard;

    struct xkb_keymap* kb_keymap;
    struct xkb_state*  kb_state;

    struct ext_session_lock_manager_v1* lock_manager;
    struct ext_session_lock_v1*         session_lock;
} screen_locker;

bool screen_locker_init(const lockscreen_args* args, screen_locker* sl);
bool screen_locker_dispatch(screen_locker* sl);
void screen_locker_destroy(screen_locker* sl);
