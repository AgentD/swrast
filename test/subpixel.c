#include "inputassembler.h"
#include "framebuffer.h"
#include "context.h"
#include "window.h"

#define WIDTH 640
#define HEIGHT 480

int main( void )
{
    context ctx;
    window* w;

    if( !(w = window_create( WIDTH, HEIGHT )) )
        return -1;

    context_init( &ctx );
    ctx.target = window_get_framebuffer( w );
    ctx.shader = SHADER_UNLIT;

    context_set_viewport( &ctx, 0, 0, WIDTH, HEIGHT );

    while( window_handle_events( w ) )
    {
        framebuffer_clear( ctx.target, 0, 0, 0, 0xFF );

        ia_begin( &ctx );

        ia_color( &ctx, 1.0f, 1.0f, 1.0f, 1.0f );

        ia_vertex( &ctx, -0.5f, -0.5f, 0.0f, 1.0f );
        ia_vertex( &ctx, 0.5f, -0.5f, 0.0f, 1.0f );
        ia_vertex( &ctx, -0.3f, 0.5f, 0.0f, 1.0f );

        ia_vertex( &ctx, 0.3f, 0.5f, 0.0f, 1.0f );
        ia_vertex( &ctx, 0.5f, -0.5f, 0.0f, 1.0f );
        ia_vertex( &ctx, -0.3f, 0.5f, 0.0f, 1.0f );

        ia_end( &ctx );

        window_display_framebuffer( w );
    }

    window_destroy( w );
    return 0;
}

