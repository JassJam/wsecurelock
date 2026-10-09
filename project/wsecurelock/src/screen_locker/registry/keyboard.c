#include "keyboard.h"

#include "screen_locker/screen_locker.h"
#include "screen_locker/draw.h"
#include "screen_locker/logger.h"

#include <ctype.h>
#include <unistd.h>
#include <sys/mman.h>

#include <wayland-client-core.h>
#include <wayland-client-protocol.h>

#include <xkbcommon/xkbcommon.h>

static void sl_on_wl_kb_keymap(void*               user_data,
                               struct wl_keyboard* wl_keyboard,
                               uint32_t            format,
                               int32_t             fd,
                               uint32_t            size)
{
    screen_locker* sl = (screen_locker*) user_data;

    if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1)
    {
        close(fd);
        return;
    }

    void* map = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    if (map == MAP_FAILED)
    {
        return;
    }

    xkb_state_unref(sl->kb_state);
    xkb_keymap_unref(sl->kb_keymap);

    sl->kb_keymap = xkb_keymap_new_from_string(sl->kb_context,
                                               (const char*) map,
                                               XKB_KEYMAP_FORMAT_TEXT_V1,
                                               XKB_KEYMAP_COMPILE_NO_FLAGS);

    munmap(map, size);
    sl->kb_state = sl->keyboard ? xkb_state_new(sl->kb_keymap) : NULL;
}

static void sl_on_wl_kb_enter(void*               user_data,
                              struct wl_keyboard* keyboard,
                              uint32_t            serial,
                              struct wl_surface*  surface,
                              struct wl_array*    keys)
{
}

static void sl_on_wl_kb_leave(void*               user_data,
                              struct wl_keyboard* keyboard,
                              uint32_t            serial,
                              struct wl_surface*  surface)
{
}

static void sl_on_wl_kb_key(void*               user_data,
                            struct wl_keyboard* keyboard,
                            uint32_t            serial,
                            uint32_t            time,
                            uint32_t            key,
                            uint32_t            state)
{
    screen_locker* sl = (screen_locker*) user_data;

    if (!sl->kb_state || state != WL_KEYBOARD_KEY_STATE_PRESSED)
    {
        return;
    }

    const xkb_keycode_t code = key + 8;
    const xkb_keysym_t  sym  = xkb_state_key_get_one_sym(sl->kb_state, code);

    switch (sym)
    {
    case XKB_KEY_Return:
    case XKB_KEY_KP_Enter:
        if (password_is_empty(&sl->pw))
        {
            return;
        }

        sl->ui.state = SL_UI_VERIFY;
        screen_locker_redraw_all(sl);
        {
            if (password_authenticate(&sl->pw))
            {
                sl->ui.state = SL_UI_UNLOCKED;
                password_reset(&sl->pw);
            }
            else
            {
                sl->ui.state = SL_UI_FAIL;
                screen_locker_redraw_all(sl);
            }
        }
        return;
    case XKB_KEY_BackSpace: password_pop(&sl->pw); break;
    case XKB_KEY_Escape   : password_reset(&sl->pw); break;
    default:
    {
        char buf[8];
        int  n = xkb_state_key_get_utf8(sl->kb_state, code, buf, sizeof buf);
        if (n > 0 && !iscntrl((unsigned char) buf[0]))
        {
            password_append(&sl->pw, buf, n);
        }
        else
        {
            return;
        }
    }
    }

    sl->ui.state = sl->pw.length ? SL_UI_TYPING : SL_UI_IDLE;
    screen_locker_redraw_all(sl);
}

static void sl_on_wl_kb_mods(void*               user_data,
                             struct wl_keyboard* keyboard,
                             uint32_t            serial,
                             uint32_t            mods_depressed,
                             uint32_t            mods_latched,
                             uint32_t            mods_locked,
                             uint32_t            group)
{
    screen_locker* sl = (screen_locker*) user_data;
    sl_assert(sl, "session locker is NULL");

    if (!sl->kb_context)
    {
        return;
    }

    sl_assert(sl->kb_state, "keyboard is NULL");
    xkb_state_update_mask(sl->kb_state, mods_depressed, mods_latched, mods_locked, 0, 0, group);
}

static void sl_on_wl_kb_repeat(void*               user_data,
                               struct wl_keyboard* keyboard,
                               int32_t             rate,
                               int32_t             delay)
{
}

void sl_registry_add_keyboard_listeners(screen_locker* sl)
{
    sl_assert(sl, "session locker is NULL");
    sl_assert(sl->keyboard, "keyboard is NULL");

    static const struct wl_keyboard_listener listeners = {
        .keymap      = sl_on_wl_kb_keymap,
        .enter       = sl_on_wl_kb_enter,
        .leave       = sl_on_wl_kb_leave,
        .key         = sl_on_wl_kb_key,
        .modifiers   = sl_on_wl_kb_mods,
        .repeat_info = sl_on_wl_kb_repeat,
    };

    wl_keyboard_add_listener(sl->keyboard, &listeners, sl);
}

