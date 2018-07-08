#ifndef WINDOW_H
#define WINDOW_H

#include <stddef.h>

#include "framebuffer.h"

typedef struct window window;

#ifdef __cplusplus
extern "C" {
#endif

window *window_create(size_t width, size_t height);

void window_destroy(window *wnd);

/* Return non-zero if the window is still active, zero if it got closed */
int window_handle_events(window *wnd);

void window_display_framebuffer(window *wnd);

framebuffer *window_get_framebuffer(window *wnd);

#ifdef __cplusplus
}
#endif

#endif /* WINDOW_H */

