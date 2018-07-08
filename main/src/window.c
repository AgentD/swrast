#include "window.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

struct window {
	Atom atom_wm_delete;
	framebuffer fb;
	Display* dpy;
	XImage* img;
	Window wnd;
	GC gc;
};

window *window_create(size_t width, size_t height)
{
	window *wnd = malloc(sizeof(*wnd));
	XSizeHints hints;

	if (!wnd)
		return NULL;

	if (!framebuffer_init(&wnd->fb, width, height))
		goto fail;

	wnd->dpy = XOpenDisplay(0);
	if (!wnd->dpy)
		goto fail_fb;

	wnd->wnd = XCreateSimpleWindow(wnd->dpy, DefaultRootWindow(wnd->dpy),
					0, 0, width, height, 0, 0, 0);
	if (!wnd->wnd)
		goto fail_dpy;

	memset(&hints, 0, sizeof(hints));
	hints.flags = PSize | PMinSize | PMaxSize;
	hints.min_width = hints.max_width = hints.base_width = width;
	hints.min_height = hints.max_height = hints.base_height = height;

	XSetWMNormalHints(wnd->dpy, wnd->wnd, &hints);

	wnd->atom_wm_delete = XInternAtom(wnd->dpy, "WM_DELETE_WINDOW", True);

	XSelectInput(wnd->dpy, wnd->wnd, StructureNotifyMask | KeyReleaseMask);
	XSetWMProtocols(wnd->dpy, wnd->wnd, &(wnd->atom_wm_delete), 1);
	XFlush(wnd->dpy);

	XStoreName(wnd->dpy, wnd->wnd, "SW Rasterizer");
	XMapWindow(wnd->dpy, wnd->wnd);

	wnd->gc = XCreateGC(wnd->dpy, wnd->wnd, 0, NULL);
	if (!wnd->gc)
		goto fail_wnd;

	wnd->img = XCreateImage(wnd->dpy, NULL, 24, ZPixmap, 0, 0,
				width, height, 32, 0);
	if (!wnd->img)
		goto fail_gc;

	return wnd;
fail_gc:
	XFreeGC(wnd->dpy, wnd->gc);
fail_wnd:
	XDestroyWindow(wnd->dpy, wnd->wnd);
fail_dpy:
	XCloseDisplay(wnd->dpy);
fail_fb:
	framebuffer_cleanup(&wnd->fb);
fail:
	free(wnd);
	return NULL;
}

void window_destroy(window *wnd)
{
	XDestroyImage(wnd->img);
	XFreeGC(wnd->dpy, wnd->gc);
	XDestroyWindow(wnd->dpy, wnd->wnd);
	XCloseDisplay(wnd->dpy);
	framebuffer_cleanup(&wnd->fb);
	free(wnd);
}

int window_handle_events(window *wnd)
{
	XEvent e;

	if (XPending(wnd->dpy)) {
		XNextEvent(wnd->dpy, &e);

		switch (e.type) {
		case ClientMessage:
			if (e.xclient.data.l[0] == (long)wnd->atom_wm_delete) {
				XUnmapWindow(wnd->dpy, wnd->wnd);
				return 0;
			}
			break;
		}
	}
	return 1;
}

void window_display_framebuffer(window *wnd)
{
	struct timespec tim;
	struct timespec tim2;

	/* copy framebuffer data */
	wnd->img->data = (char*)wnd->fb.color;
	XPutImage(wnd->dpy, wnd->wnd, wnd->gc, wnd->img,
		0, 0, 0, 0, wnd->fb.width, wnd->fb.height);

	wnd->img->data = NULL;

	/* wait for ~16.666 ms -> ~60 fps */
	tim.tv_sec = 0;
	tim.tv_nsec = 16666666L;
	nanosleep(&tim, &tim2);
}

framebuffer *window_get_framebuffer(window *wnd)
{
	return &wnd->fb;
}
