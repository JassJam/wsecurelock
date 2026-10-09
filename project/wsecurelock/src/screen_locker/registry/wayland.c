#include "wayland.h"
#include "session.h"
#include "display.h"
#include "seat.h"
#include "screen_locker/screen_locker.h"

#include "ext-session-lock-v1-client-protocol.h"
#include "screen_locker/logger.h"

#include <stdlib.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>

#include <string.h>

static void on_interface_added(void*               user_data,
                               struct wl_registry* registry,
                               uint32_t            name,
                               const char*         interface,
                               uint32_t            version)
{
    screen_locker* sl = (screen_locker*) user_data;

    if (!strcmp(interface, wl_compositor_interface.name))
    {
        sl->compositor = (struct wl_compositor*)
            wl_registry_bind(registry, name, &wl_compositor_interface, version);
    }
    else if (!strcmp(interface, wl_shm_interface.name))
    {
        sl->shared_memory =
            (struct wl_shm*) wl_registry_bind(registry, name, &wl_shm_interface, version);
    }
    else if (!strcmp(interface, wl_seat_interface.name) && !sl->seat)
    {
        sl_registry_add_seat_listeners(sl, registry, name, version);
    }
    else if (!strcmp(interface, ext_session_lock_manager_v1_interface.name))
    {
        sl_register_add_session_listeners(sl, registry, name, version);
    }
    else if (!strcmp(interface, wl_output_interface.name))
    {
        sl_registry_add_display_listeners(sl, registry, name, version);
    }
}

static void on_interface_removed(void* d, struct wl_registry* reg, uint32_t name)
{
}

void sl_registry_add_wayland_listeners(struct screen_locker* sl)
{
    sl_assert(sl, "session locker is NULL");
    sl_assert(sl->display, "session display is NULL");

    static struct wl_registry_listener listeners = {
        .global        = on_interface_added,
        .global_remove = on_interface_removed,
    };

    struct wl_registry* registry = wl_display_get_registry(sl->display);
    wl_registry_add_listener(registry, &listeners, sl);
    wl_display_roundtrip(sl->display);

    if (!sl->compositor || !sl->shared_memory || !sl->seat || !sl->display)
    {
        sl_print_error("Failed to initialize lockscreen");
        exit(EXIT_FAILURE);
    }
}
