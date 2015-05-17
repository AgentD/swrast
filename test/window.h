/**
 * \file window.h
 *
 * \brief Contains the window object implementation for testing
 */
#ifndef WINDOW_H
#define WINDOW_H



#include "framebuffer.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>



/**
 * \struct window
 *
 * \brief A simple Xlib based window for testing
 */
typedef struct
{
    Atom atom_wm_delete;
    framebuffer fb;
    Display* dpy;
    XImage* img;
    Window wnd;
    GC gc;
}
window;



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Create a simple X11 window for test drawing
 *
 * \memberof window
 *
 * \param width  The width of the window drawing area
 * \param height The height of the window drawing area
 *
 * \return A pointer to a window structure on success, NULL on failure
 */
window* window_create( unsigned int width, unsigned int height );

/**
 * \brief Destroy a window created with window_create and free its resources
 *
 * \memberof window
 *
 * \param wnd A pointer to a window structure
 */
void window_destroy( window* wnd );

/**
 * \brief Fetch and handle window system messages for a window
 *
 * \memberof window
 *
 * \param wnd A pointer to a window structure
 *
 * \return Non-zero if the window is still active, zero if it got closed
 */
int window_handle_events( window* wnd );

/**
 * \brief Copy the contents of a windows framebuffer to the window
 *
 * \memberof window
 *
 * \param wnd A pointer to a window structure
 */
void window_display_framebuffer( window* wnd );

#ifdef __cplusplus
}
#endif

#endif /* WINDOW_H */

