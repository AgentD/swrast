#include "framebuffer.h"
#include "rasterizer.h"
#include "transform.h"
#include "texture.h"
#include "window.h"

#include <math.h>



int main( void )
{
    float a = 0.0f, c, s, m[16], p[16], far, near, aspect, f, iNF;
    rasterizer_state rs;
    unsigned char* ptr;
    triangle t0, t1, t;
    unsigned int x, y;
    framebuffer* fb;
    texture* tex;
    window* w;

    /************* initalisation *************/
    if( !(fb = framebuffer_create( 640, 480 )) )
        return -1;

    if( !(w = window_create( fb->width, fb->height )) )
    {
        framebuffer_destroy( fb );
        return -1;
    }

    t0.v0.x = -2.0f; t0.v0.y = -2.0f; t0.v0.z = 0.0f; t0.v0.w = 1.0f;
    t0.v1.x =  2.0f; t0.v1.y = -2.0f; t0.v1.z = 0.0f; t0.v1.w = 1.0f;
    t0.v2.x =  0.0f; t0.v2.y =  2.0f; t0.v2.z = 0.0f; t0.v2.w = 1.0f;

    t0.v0.s = 0.0; t0.v0.t = 1.0;
    t0.v1.s = 1.0; t0.v1.t = 1.0;
    t0.v2.s = 0.5; t0.v2.t = 0.0;

    t0.v0.r = 0xFF; t0.v0.g = 0x00; t0.v0.b = 0x00; t0.v0.a = 0xFF;
    t0.v1.r = 0x00; t0.v1.g = 0xFF; t0.v1.b = 0x00; t0.v1.a = 0xFF;
    t0.v2.r = 0x00; t0.v2.g = 0x00; t0.v2.b = 0xFF; t0.v2.a = 0xFF;

    t1.v0.x = 0.0f; t1.v0.y = -2.0f; t1.v0.z = -2.0f; t1.v0.w = 1.0f;
    t1.v1.x = 0.0f; t1.v1.y = -2.0f; t1.v1.z =  2.0f; t1.v1.w = 1.0f;
    t1.v2.x = 0.0f; t1.v2.y =  2.0f; t1.v2.z =  0.0f; t1.v2.w = 1.0f;

    t1.v0.r = 0xFF; t1.v0.g = 0xFF; t1.v0.b = 0x00; t1.v0.a = 0x80;
    t1.v1.r = 0xFF; t1.v1.g = 0xFF; t1.v1.b = 0x00; t1.v1.a = 0x80;
    t1.v2.r = 0xFF; t1.v2.g = 0xFF; t1.v2.b = 0x00; t1.v2.a = 0x80;

    /* intialize projection matrix */
    far    = 0.5f;
    near   = 500.0f;
    aspect = ((float)fb->width) / ((float)fb->height);
    f      = 1.0 / tan( 60.0f * (3.14159265359f/180.0f) * 0.5f );
	iNF    = 1.0 / ( near - far );

    p[0]=f/aspect; p[4]=0.0f; p[ 8]= 0.0f;           p[12]=0.0f;
    p[1]=0.0f;     p[5]=f;    p[ 9]= 0.0f;           p[13]=0.0f;
    p[2]=0.0f;     p[6]=0.0f; p[10]= (far+near)*iNF; p[14]=2.0f*far*near*iNF;
    p[3]=0.0f;     p[7]=0.0f; p[11]=-1.0f;           p[15]=0.0f;

    /* create and initialize texture */
    tex = texture_create( 64, 64 );

    if( tex )
    {
        for( ptr=tex->data, y=0; y<tex->height; ++y )
        {
            for( x=0; x<tex->width; ++x, ptr+=4 )
            {
                ptr[0]=ptr[1]=ptr[2] = ((y&0x08) ^ (x&0x08)) ? 0x00 : 0xFF;
                ptr[3] = 0xFF;
            }
        }
    }

    /* initialize rasterizer state */
    rs.alpha_blend       = 1;
    rs.depth_test        = 1;
    rs.texture_enable[0] = 0;
    rs.textures[0]       = tex;
    rasterizer_set_state( &rs );

    /************* drawing loop *************/
    while( window_handle_events( w ) )
    {
        /* clear framebuffer */
        framebuffer_clear( fb, 0, 0, 0, 0xFF );
        framebuffer_clear_depth( fb, 1.0 );

        /* create transformation matrix */
        c = cos( a ), s = sin( a );
        a += 0.02f;

        m[0] =    c; m[4] = 0.0f; m[ 8] =    s; m[12] =  0.0f;
        m[1] = 0.0f; m[5] = 1.0f; m[ 9] = 0.0f; m[13] =  0.0f;
        m[2] =   -s; m[6] = 0.0f; m[10] =    c; m[14] =-10.0f;
        m[3] = 0.0f; m[7] = 0.0f; m[11] = 0.0f; m[15] =  1.0f;

        /* rasterize triangles */
        rs.texture_enable[0] = 1;
        rasterizer_set_state( &rs );

        triangle_transform( &t0, &t, m );
        triangle_transform( &t, &t, p );
        triangle_perspective_divide( &t );
        triangle_rasterize( &t, fb );

        rs.texture_enable[0] = 0;
        rasterizer_set_state( &rs );

        triangle_transform( &t1, &t, m );
        triangle_transform( &t, &t, p );
        triangle_perspective_divide( &t );
        triangle_rasterize( &t, fb );

        /* copy to window */
        window_display_framebuffer( w, fb );
    }

    /************* cleanup *************/
    texture_destroy( tex );
    framebuffer_destroy( fb );
    window_destroy( w );
    return 0;
}

