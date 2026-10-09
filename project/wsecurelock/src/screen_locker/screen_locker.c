#include "screen_locker/screen_locker.h"

#include "registry/session.h"
#include "registry/wayland.h"
#include "screen_locker/logger.h"

#include <stdio.h>
#include <unistd.h>
#include <pwd.h>
#include <string.h>

#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>

//

static bool sl_init_user(screen_locker* sl)
{
    struct passwd* pw = getpwuid(getuid());
    if (!pw)
    {
        return false;
    }

    snprintf(sl->user, sizeof(sl->user), "%s", pw->pw_name);
    return true;
}

static bool sl_init_wayland(screen_locker* sl)
{
    sl->display = wl_display_connect(NULL);
    if (!sl->display)
    {
        return false;
    }

    sl->kb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (!sl->kb_context)
    {
        return false;
    }

    sl_registry_add_wayland_listeners(sl);
    return true;
}

//

bool screen_locker_init(const lockscreen_args* args, screen_locker* sl)
{
    if (!args || !sl)
    {
        sl_print_error("Invalid arguments");
        return false;
    }

    memset(sl, 0, sizeof(*sl));
    if (!sl_init_user(sl))
    {
        sl_print_error("cannot determine user");
        return false;
    }

    if (!sl_init_wayland(sl))
    {
        sl_print_error("cannot connect to wayland/xkb context");
        return false;
    }

    sl_lock_session(sl);
    return true;
}

bool screen_locker_dispatch(screen_locker* sl)
{
    if (sl->ui.state == SL_UI_UNLOCKED)
    {
        return false;
    }

    return wl_display_dispatch(sl->display) != -1;
}

void screen_locker_destroy(screen_locker* sl)
{
    if (sl->ui.state == SL_UI_UNLOCKED)
    {
        sl_unlock_session(sl);
    }
}
