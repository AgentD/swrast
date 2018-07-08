#include "inputassembler.h"
#include "framebuffer.h"
#include "rasterizer.h"
#include "texture.h"
#include "context.h"
#include "window.h"
#include "vector.h"
#include "color.h"
#include "3ds.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#define WIDTH 1024
#define HEIGHT 768

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

    ctx.flags &= ~(CULL_FRONT|CULL_BACK);
    ctx.vertex_format = 0;
    ctx.vertexbuffer = NULL;
    ctx.texture_enable[0] = 1;
    ctx.textures[0] = tex;

    ctx.shader = shader_internal(SHADER_UNLIT);
    context_set_modelview_matrix( &ctx, m );

    ia_begin( &ctx );

    ia_color( &ctx, 1.0f, 0.0f, 0.0f, 1.0f );
    ia_texcoord( &ctx, 0, 0.0f, 1.0f );
    ia_vertex( &ctx, -2.0f, -2.0f, 0.0f, 1.0f );

    ia_color( &ctx, 0.0f, 1.0f, 0.0f, 1.0f );
    ia_texcoord( &ctx, 0, 1.0f, 1.0f );
    ia_vertex( &ctx, 2.0f, -2.0f, 0.0f, 1.0f );

    ia_color( &ctx, 0.0f, 0.0f, 1.0f, 1.0f );
    ia_texcoord( &ctx, 0, 0.5f, 0.0f );
    ia_vertex( &ctx, 0.0f, 2.0f, 0.0f, 1.0f );

    ia_end( &ctx );

    /* yellow transparent triangle */
    ctx.texture_enable[0] = 0;
    ctx.textures[0] = NULL;
    ctx.flags |= BLEND_ENABLE;

    ia_begin( &ctx );
    ia_color( &ctx, 1.0f, 1.0f, 0.0f, 0.5f );
    ia_vertex( &ctx, 0.0f, -2.0f, -2.0f, 1.0f );
    ia_vertex( &ctx, 0.0f, -2.0f, 2.0f, 1.0f );
    ia_vertex( &ctx, 0.0f, 2.0f, 0.0f, 1.0f );
    ia_end( &ctx );

    ctx.shader = shader_internal(SHADER_PHONG);

    /* rasterize teapot */
    m[0] =    c*0.05f; m[4] = 0.0f;  m[ 8] =    s*0.05f; m[12] =  2.0f;
    m[1] = 0.0f;       m[5] = 0.05f; m[ 9] = 0.0f;       m[13] = -1.0f;
    m[2] =   -s*0.05f; m[6] = 0.0f;  m[10] =    c*0.05f; m[14] = -5.0f;
    m[3] = 0.0f;       m[7] = 0.0f;  m[11] = 0.0f;       m[15] =  1.0f;

    ctx.flags            &= ~BLEND_ENABLE;
    ctx.flags            |= CULL_BACK;
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
    framebuffer *fb;
    window* w;

    /************* initalisation *************/
    if( !(w = window_create( WIDTH, HEIGHT )) )
        return -1;

    teapot = load_3ds( "teapot.3ds" );

    context_init( &ctx );

    /* intialize projection matrix */
    far    = 0.5f;
    near   = 500.0f;
    aspect = (float)WIDTH / (float)HEIGHT;
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
    ctx.depth_test = COMPARE_LESS_EQUAL;
    ctx.flags |= DEPTH_TEST|FRONT_CCW;

    ctx.light[0].enable = 1;
    ctx.light[0].diffuse = vec4_set( 1.0f, 1.0f, 1.0f, 1.0f );
    ctx.light[0].specular = vec4_set( 1.0f, 1.0f, 1.0f, 1.0f );
    ctx.light[0].attenuation_constant = 1.0f;

    ctx.material.diffuse = vec4_set( 0.5f, 0.5f, 0.5f, 1.0f );
    ctx.material.specular = vec4_set( 0.5f, 0.5f, 0.5f, 1.0f );
    ctx.material.ambient = vec4_set( 0.0f, 0.0f, 0.0f, 1.0f );
    ctx.material.emission = vec4_set( 0.0f, 0.0f, 0.0f, 1.0f );
    ctx.material.shininess = 127;

    fb = window_get_framebuffer( w );
    ctx.target = fb;

    context_set_viewport( &ctx, 0, 0, WIDTH, HEIGHT );

    /************* drawing loop *************/
    while( window_handle_events( w ) )
    {
        framebuffer_clear( fb, 0, 0, 0, 0xFF );
        framebuffer_clear_depth( fb, 1.0 );

        draw_scene( );

        a += 0.02f;

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

