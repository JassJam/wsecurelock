#include "display.h"
#include "screen_locker/screen_locker.h"
#include "screen_locker/draw.h"

#include "ext-session-lock-v1-client-protocol.h"
#include "screen_locker/logger.h"

#include <stdlib.h>

static void on_configure(void*                               user_data,
                         struct ext_session_lock_surface_v1* ls,
                         uint32_t                            serial,
                         uint32_t                            w,
                         uint32_t                            h)
{
    struct screen_locker_output* output = (struct screen_locker_output*) user_data;

    ext_session_lock_surface_v1_ack_configure(ls, serial);

    if (w != output->w || h != output->h || !output->bg)
    {
        output->w = w;
        output->h = h;
        screen_locker_output_fill_bg(output);
    }

    screen_locker_redraw(output);
}

void sl_registry_lock_display(screen_locker_output* output)
{
    const screen_locker* sl = output->parent;

    sl_assert(sl, "session locker is NULL");
    sl_assert(sl->compositor, "compositor is NULL");
    sl_assert(sl->display, "display is NULL");
    sl_assert(sl->session_lock, "session lock is NULL");

    static const struct ext_session_lock_surface_v1_listener listeners = {
        .configure = on_configure,
    };

    output->surface = wl_compositor_create_surface(sl->compositor);
    output->lock_surface =
        ext_session_lock_v1_get_lock_surface(sl->session_lock, output->surface, output->display);
    ext_session_lock_surface_v1_add_listener(output->lock_surface, &listeners, output);
}

void sl_registry_add_display_listeners(screen_locker*      sl,
                                       struct wl_registry* registry,
                                       uint32_t            name,
                                       uint32_t            version)
{
    sl_assert(sl, "session locker is NULL");

    screen_locker_output* output =
        (screen_locker_output*) calloc(1, sizeof(struct screen_locker_output));

    output->parent  = sl;
    output->display = wl_registry_bind(registry, name, &wl_output_interface, version);
    output->next    = sl->ui.outputs;
    sl->ui.outputs  = output;

    // we are on a locked state
    // and a new monitor has been plugged in
    // create a surface for it and lock it
    if (sl->ui.state != SL_UI_UNLOCKED)
    {
        sl_registry_lock_display(output);
    }
}
