#include "window.h"

#include <stdlib.h>
#include <unistd.h>



window* window_create( unsigned int width, unsigned int height )
{
    XSizeHints hints;
    window* wnd;

    wnd = malloc( sizeof(window) );

    if( !wnd )
        return NULL;

    /********** open X11 display connection **********/
    wnd->dpy = XOpenDisplay( 0 );

    if( !wnd->dpy )
    {
        free( wnd );
        return NULL;
    }

    /********** create a window **********/
    wnd->wnd = XCreateSimpleWindow( wnd->dpy, DefaultRootWindow( wnd->dpy ),
                                    0, 0, width, height, 0, 0, 0 );

    if( !wnd->wnd )
    {
        XCloseDisplay( wnd->dpy );
        free( wnd );
        return NULL;
    }

    /********** make the window non resizeable **********/
    hints.flags = PSize | PMinSize | PMaxSize;
    hints.min_width = hints.max_width = hints.base_width = width;
    hints.min_height = hints.max_height = hints.base_height = height;

    XSetWMNormalHints( wnd->dpy, wnd->wnd, &hints );

    /********** tell the X server what events we will handle **********/
    wnd->atom_wm_delete = XInternAtom( wnd->dpy, "WM_DELETE_WINDOW", True );

    XSelectInput( wnd->dpy, wnd->wnd, StructureNotifyMask | KeyReleaseMask );
    XSetWMProtocols( wnd->dpy, wnd->wnd, &(wnd->atom_wm_delete), 1 );
    XFlush( wnd->dpy );

    /********** make the window visible **********/
    XStoreName( wnd->dpy, wnd->wnd, "SW Rasterizer" );
    XMapWindow( wnd->dpy, wnd->wnd );

    /********** create a graphics context **********/
    wnd->gc = XCreateGC( wnd->dpy, wnd->wnd, 0, NULL );

    if( !wnd->gc )
    {
        XDestroyWindow( wnd->dpy, wnd->wnd );
        XCloseDisplay( wnd->dpy );
        free( wnd );
        return NULL;
    }

    /********** create XImage structure **********/
    wnd->img = XCreateImage( wnd->dpy, NULL, 24, ZPixmap, 0, 0,
			                 width, height, 32, 0 );

    if( !wnd->img )
    {
        XFreeGC( wnd->dpy, wnd->gc );
        XDestroyWindow( wnd->dpy, wnd->wnd );
        XCloseDisplay( wnd->dpy );
        free( wnd );
        return NULL;
    }

    return wnd;
}

void window_destroy( window* wnd )
{
    if( wnd )
    {
        XDestroyImage( wnd->img );
        XFreeGC( wnd->dpy, wnd->gc );
        XDestroyWindow( wnd->dpy, wnd->wnd );
        XCloseDisplay( wnd->dpy );
        free( wnd );
    }
}

int window_handle_events( window* wnd )
{
    XEvent e;

    if( XPending( wnd->dpy ) )
    {
        XNextEvent( wnd->dpy, &e );

        switch( e.type )
        {
        case ClientMessage:
            if( e.xclient.data.l[0] == (long)wnd->atom_wm_delete )
            {
                XUnmapWindow( wnd->dpy, wnd->wnd );
                return 0;
            }
            break;
        }
    }
    return 1;
}

void window_display_framebuffer( window* wnd, framebuffer* fb )
{
    /* copy framebuffer data */
    wnd->img->data = (char*)fb->color;
    XPutImage( wnd->dpy, wnd->wnd, wnd->gc, wnd->img,
               0, 0, 0, 0, fb->width, fb->height );
    wnd->img->data = NULL;

    /* wait for ~16.666 ms -> ~60 fps */
    usleep( 16666 );
}

