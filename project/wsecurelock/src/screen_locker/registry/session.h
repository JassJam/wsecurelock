#pragma once

#include <stdint.h>

struct screen_locker;
struct wl_registry;

void sl_register_add_session_listeners(struct screen_locker* sl,
                                       struct wl_registry*   registry,
                                       uint32_t              name,
                                       uint32_t              version);

void sl_lock_session(struct screen_locker* sl);

void sl_unlock_session(struct screen_locker* sl);
