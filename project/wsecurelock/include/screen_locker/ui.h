#pragma once

#include "lockscreen_args.h"

#include <stdint.h>

typedef enum ui_state_type
{
    SL_UI_UNLOCKED,
    SL_UI_IDLE,
    SL_UI_TYPING,
    SL_UI_VERIFY,
    SL_UI_FAIL,
} ui_state_type;

typedef struct screen_locker_output
{
    struct wl_output*                   display;
    struct wl_surface*                  surface;
    struct ext_session_lock_surface_v1* lock_surface;

    struct screen_locker_output* next;
    struct screen_locker*        parent;

    uint32_t  w, h;
    uint32_t* bg;
} screen_locker_output;

typedef struct ui_state
{
    lockscreen_args       args;
    ui_state_type         state;
    screen_locker_output* outputs;
} screen_locker_ui;
