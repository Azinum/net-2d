// platform.h

#ifndef _PLATFORM_H
#define _PLATFORM_H

struct Render_state;

i32 platform_open_window(i32 width, i32 height, const char* title);

i32 platform_process_events();

void platform_swap_buffers(struct Render_state* renderer);

void platform_set_window_title(const char* title);

void platform_close_window();

#endif
