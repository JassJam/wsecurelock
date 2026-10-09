#pragma once

#include <stdint.h>

struct screen_locker;
struct screen_locker_output;
struct wl_registry;

void sl_registry_lock_display(struct screen_locker_output* output);

void sl_registry_add_display_listeners(struct screen_locker* sl,
                                       struct wl_registry*   registry,
                                       uint32_t              name,
                                       uint32_t              version);
