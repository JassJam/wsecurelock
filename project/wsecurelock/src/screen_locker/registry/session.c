#include "session.h"
#include "display.h"
#include "screen_locker/screen_locker.h"
#include "screen_locker/logger.h"

#include "ext-session-lock-v1-client-protocol.h"

#include <stdio.h>
#include <stdlib.h>

static void on_lock_added(void* user_data, struct ext_session_lock_v1* session_lock)
{
    screen_locker* sl = (screen_locker*) user_data;
    sl->ui.state      = SL_UI_IDLE;
}

static void on_lock_finished(void* user_data, struct ext_session_lock_v1* session_lock)
{
    sl_print_error("compositor refused the lock, possibly another lock is already running");
    exit(EXIT_FAILURE);
}

void sl_register_add_session_listeners(screen_locker*      sl,
                                       struct wl_registry* registry,
                                       uint32_t            name,
                                       uint32_t            version)
{
    sl->lock_manager = (struct ext_session_lock_manager_v1*)
        wl_registry_bind(registry, name, &ext_session_lock_manager_v1_interface, version);
}

void sl_lock_session(struct screen_locker* sl)
{
    sl_assert(sl, "session locker is NULL");
    sl_assert(sl->lock_manager, "session lock manager is NULL");
    sl_assert(sl->display, "session display is NULL");

    static const struct ext_session_lock_v1_listener listeners = {
        .locked   = on_lock_added,
        .finished = on_lock_finished,
    };

    sl->session_lock = ext_session_lock_manager_v1_lock(sl->lock_manager);
    ext_session_lock_v1_add_listener(sl->session_lock, &listeners, sl);

    for (screen_locker_output* output = sl->ui.outputs; output; output = output->next)
    {
        sl_registry_lock_display(output);
    }

    // wait til we initially exit our SL_UI_UNLOCKED state
    while (sl->ui.state == SL_UI_UNLOCKED)
    {
        if (wl_display_roundtrip(sl->display) < 0)
        {
            sl_print_error("Failed to lockdown session");
            exit(EXIT_FAILURE);
        }
    }
}

void sl_unlock_session(struct screen_locker* sl)
{
    sl_assert(sl, "session locker is NULL");
    sl_assert(sl->lock_manager, "session lock manager is NULL");
    sl_assert(sl->display, "session display is NULL");

    ext_session_lock_v1_unlock_and_destroy(sl->session_lock);
    wl_display_roundtrip(sl->display);
}
