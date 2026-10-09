#pragma once

#include <stdint.h>

struct screen_locker;
struct wl_registry;

void sl_registry_add_seat_listeners(struct screen_locker* sl,
                                    struct wl_registry*   registry,
                                    uint32_t              name,
                                    uint32_t              version);
