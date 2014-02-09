#include "inputassembler.h"
#include "framebuffer.h"
#include "rasterizer.h"
#include "texture.h"
#include "window.h"

#include <math.h>



float vbo[60] =
{
    /* colorfull opaque triangle */
    -2.0f, -2.0f,  0.0f,  1.0f,
     1.0f,  0.0f,  0.0f,  1.0f,
     0.0f,  1.0f,

     2.0f, -2.0f,  0.0f,  1.0f,
     0.0f,  1.0f,  0.0f,  1.0f,
     1.0f,  1.0f,

     0.0f,  2.0f,  0.0f,  1.0f,
     0.0f,  0.0f,  1.0f,  1.0f,
     0.5f,  0.0f,

    /* yellow transparent triangle */
     0.0f, -2.0f, -2.0f,  1.0f,
     1.0f,  1.0f,  0.0f,  0.5f,
     0.0f,  0.0f,

     0.0f, -2.0f,  2.0f,  1.0f,
     1.0f,  1.0f,  0.0f,  0.5f,
     0.0f,  0.0f,

     0.0f,  2.0f,  0.0f,  1.0f,
     1.0f,  1.0f,  0.0f,  0.5f,
     0.0f,  0.0f
};

int main( void )
{
    float a = 0.0f, c, s, m[16], far, near, aspect, f, iNF;
    rasterizer_state rs;
    unsigned char* ptr;
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

    /* intialize projection matrix */
    far    = 0.5f;
    near   = 500.0f;
    aspect = ((float)fb->width) / ((float)fb->height);
    f      = 1.0 / tan( 60.0f * (3.14159265359f/180.0f) * 0.5f );
	iNF    = 1.0 / ( near - far );

    m[0]=f/aspect; m[4]=0.0f; m[ 8]= 0.0f;           m[12]=0.0f;
    m[1]=0.0f;     m[5]=f;    m[ 9]= 0.0f;           m[13]=0.0f;
    m[2]=0.0f;     m[6]=0.0f; m[10]= (far+near)*iNF; m[14]=2.0f*far*near*iNF;
    m[3]=0.0f;     m[7]=0.0f; m[11]=-1.0f;           m[15]=0.0f;

    ia_set_projection_matrix( m );

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

    /* initialize state */
    rs.alpha_blend       = 1;
    rs.depth_test        = 1;
    rs.texture_enable[0] = 1;
    rs.textures[0]       = tex;
    rasterizer_set_state( &rs );

    ia_set_vertex_format( VF_POSITION_F4 | VF_COLOR_F4 | VF_TEX0 );

    /************* drawing loop *************/
    while( window_handle_events( w ) )
    {
        /* clear framebuffer */
        framebuffer_clear( fb, 0, 0, 0, 0xFF );
        framebuffer_clear_depth( fb, 1.0 );

        /* update modelview matrix */
        c = cos( a ), s = sin( a );
        a += 0.02f;

        m[0] =    c; m[4] = 0.0f; m[ 8] =    s; m[12] =  0.0f;
        m[1] = 0.0f; m[5] = 1.0f; m[ 9] = 0.0f; m[13] =  0.0f;
        m[2] =   -s; m[6] = 0.0f; m[10] =    c; m[14] =-10.0f;
        m[3] = 0.0f; m[7] = 0.0f; m[11] = 0.0f; m[15] =  1.0f;

        ia_set_modelview_matrix( m );

        /* rasterize triangles */
        ia_draw_triangles( fb, vbo, 6 );

        /* copy to window */
        window_display_framebuffer( w, fb );
    }

    /************* cleanup *************/
    texture_destroy( tex );
    framebuffer_destroy( fb );
    window_destroy( w );
    return 0;
}

