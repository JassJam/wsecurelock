#pragma once

struct screen_locker;
struct screen_locker_output;

void screen_locker_output_fill_bg(struct screen_locker_output* output);

void screen_locker_redraw(struct screen_locker_output* output);
void screen_locker_redraw_all(struct screen_locker* sl);
