#include "inputassembler.h"
#include "framebuffer.h"
#include "rasterizer.h"
#include "texture.h"
#include "context.h"
#include "window.h"
#include "3ds.h"
#include "tl.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>



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

static context ctx;
static float a = 0.0f;
static texture* tex;
static mesh* teapot;

static void draw_scene( void )
{
    float c, s, m[16];

    /* rasterize triangles */
    c = cos( a ), s = sin( a );

    m[0] = c;    m[4] = 0.0f; m[ 8] =    s; m[12] =   0.0f;
    m[1] = 0.0f; m[5] = 1.0f; m[ 9] = 0.0f; m[13] =   0.0f;
    m[2] = -s;   m[6] = 0.0f; m[10] =    c; m[14] = -10.0f;
    m[3] = 0.0f; m[7] = 0.0f; m[11] = 0.0f; m[15] =   1.0f;

    ctx.alpha_blend       = 1;
    ctx.depth_test        = COMPARE_LESS_EQUAL;
    ctx.texture_enable[0] = 1;
    ctx.textures[0]       = tex;
    ctx.cull_cw           = 0;
    ctx.cull_ccw          = 0;
    ctx.light_enable      = 0;
    ctx.vertex_format     = VF_POSITION_F4 | VF_COLOR_F4 | VF_TEX0;
    ctx.vertexbuffer      = vbo;

    context_set_modelview_matrix( &ctx, m );
    ia_draw_triangles( &ctx, 6 );

    /* rasterize teapot */
    m[0] =    c*0.05f; m[4] = 0.0f;  m[ 8] =    s*0.05f; m[12] =  2.0f;
    m[1] = 0.0f;       m[5] = 0.05f; m[ 9] = 0.0f;       m[13] = -1.0f;
    m[2] =   -s*0.05f; m[6] = 0.0f;  m[10] =    c*0.05f; m[14] = -5.0f;
    m[3] = 0.0f;       m[7] = 0.0f;  m[11] = 0.0f;       m[15] =  1.0f;

    ctx.alpha_blend       = 0;
    ctx.depth_test        = COMPARE_LESS_EQUAL;
    ctx.texture_enable[0] = 0;
    ctx.textures[0]       = 0;
    ctx.cull_cw           = 1;
    ctx.cull_ccw          = 0;
    ctx.light_enable      = 1;
    ctx.vertex_format     = teapot->format;
    ctx.vertexbuffer      = teapot->vertexbuffer;
    ctx.indexbuffer       = teapot->indexbuffer;

    context_set_modelview_matrix( &ctx, m );
    ia_draw_triangles_indexed( &ctx, teapot->vertices, teapot->indices );
}

int main( void )
{
    float far, near, aspect, f, iNF, m[16];
    unsigned char* ptr;
    unsigned int x, y;
    window* w;

    /************* initalisation *************/
    if( !(w = window_create( 1024, 768 )) )
        return -1;

    teapot = load_3ds( "teapot.3ds" );

    context_init( &ctx );

    /* intialize projection matrix */
    far    = 0.5f;
    near   = 500.0f;
    aspect = ((float)w->fb.width) / ((float)w->fb.height);
    f      = 1.0 / tan( 60.0f * (3.14159265359f/180.0f) * 0.5f );
	iNF    = 1.0 / ( near - far );

    m[0]=f/aspect; m[4]=0.0f; m[ 8]= 0.0f;           m[12]=0.0f;
    m[1]=0.0f;     m[5]=f;    m[ 9]= 0.0f;           m[13]=0.0f;
    m[2]=0.0f;     m[6]=0.0f; m[10]= (far+near)*iNF; m[14]=2.0f*far*near*iNF;
    m[3]=0.0f;     m[7]=0.0f; m[11]=-1.0f;           m[15]=0.0f;

    context_set_projection_matrix( &ctx, m );

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

    /* initialize T&L state */
    ctx.light[0].enable = 1;
    ctx.light[0].diffuse[0] = 1.0f;
    ctx.light[0].diffuse[1] = 1.0f;
    ctx.light[0].diffuse[2] = 1.0f;
    ctx.light[0].specular[0] = 1.0f;
    ctx.light[0].specular[1] = 1.0f;
    ctx.light[0].specular[2] = 1.0f;
    ctx.light[0].attenuation_constant = 1.0f;

    ctx.material.diffuse[0] = 0.5f;
    ctx.material.diffuse[1] = 0.5f;
    ctx.material.diffuse[2] = 0.5f;
    ctx.material.specular[0] = 0.5f;
    ctx.material.specular[1] = 0.5f;
    ctx.material.specular[2] = 0.5f;
    ctx.material.ambient[0] = 0.0f;
    ctx.material.ambient[1] = 0.0f;
    ctx.material.ambient[2] = 0.0f;
    ctx.material.emission[0] = 0.0f;
    ctx.material.emission[1] = 0.0f;
    ctx.material.emission[2] = 0.0f;
    ctx.material.shininess = 127;

    ctx.target = &w->fb;

    /************* drawing loop *************/
    while( window_handle_events( w ) )
    {
        /* clear framebuffer */
        framebuffer_clear( &w->fb, 0, 0, 0, 0xFF );
        framebuffer_clear_depth( &w->fb, 1.0 );

        /* draw center cross */
        for( x=0; x<(unsigned int)w->fb.width; ++x )
        {
            ptr = w->fb.color + ((w->fb.height/2 - 1)*w->fb.width + x)*4;
            ptr[0] = ptr[1] = ptr[2] = 0xFF;
            *(w->fb.depth + (w->fb.height/2 - 1)*w->fb.width + x) = 0.0f;
        }

        for( y=0; y<(unsigned int)w->fb.height; ++y )
        {
            ptr = w->fb.color + (y*w->fb.width + w->fb.width/2 - 1)*4;
            ptr[0] = ptr[1] = ptr[2] = 0xFF;
            *(w->fb.depth + y*w->fb.width + w->fb.width/2 - 1) = 0.0f;
        }

        /* draw scene into multiple view ports */
        context_set_viewport( &ctx, 0, 0, w->fb.width/2, w->fb.height/2 );
        ctx.shade_model = SHADE_GOURAUD;
        draw_scene( );

        context_set_viewport( &ctx, w->fb.width/2, 0,
                                    w->fb.width/2, w->fb.height/2 );
        ctx.shade_model = SHADE_FLAT;
        draw_scene( );

        context_set_viewport( &ctx, 0, w->fb.height/2,
                                    w->fb.width/2, w->fb.height/2 );
        ctx.shade_model = SHADE_GOURAUD;
        draw_scene( );

        context_set_viewport( &ctx, w->fb.width/2, w->fb.height/2,
                                    w->fb.width/2, w->fb.height/2 );
        ctx.shade_model = SHADE_FLAT;
        draw_scene( );

        a += 0.02f;

        /* copy to window */
        window_display_framebuffer( w );
    }

    /************* cleanup *************/
    texture_destroy( tex );
    window_destroy( w );
    free( teapot->vertexbuffer );
    free( teapot->indexbuffer );
    free( teapot );
    return 0;
}

