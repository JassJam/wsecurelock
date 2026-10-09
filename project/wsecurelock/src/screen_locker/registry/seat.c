#include "seat.h"
#include "keyboard.h"
#include "screen_locker/logger.h"
#include "screen_locker/screen_locker.h"

#include <wayland-client-protocol.h>

static void on_seat_caps(void* user_data, struct wl_seat* seat, uint32_t caps)
{
    screen_locker* sl = (screen_locker*) user_data;

    if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !sl->keyboard)
    {
        sl->keyboard = wl_seat_get_keyboard(seat);
        sl_registry_add_keyboard_listeners(sl);
    }
}

static void on_seat_name(void* user_data, struct wl_seat* seat, const char* name)
{
}

void sl_registry_add_seat_listeners(screen_locker*      sl,
                                    struct wl_registry* registry,
                                    uint32_t            name,
                                    uint32_t            version)
{
    sl_assert(sl, "session locker is NULL");

    static const struct wl_seat_listener listeners = {
        .capabilities = on_seat_caps,
        .name         = on_seat_name,
    };

    sl->seat = (struct wl_seat*) wl_registry_bind(registry, name, &wl_seat_interface, version);
    wl_seat_add_listener(sl->seat, &listeners, sl);
}
