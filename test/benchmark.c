#include "inputassembler.h"
#include "framebuffer.h"
#include "context.h"
#include "3ds.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>



static mesh* teapot;



static double get_time( void )
{
    struct timespec ts;
    clock_gettime( CLOCK_MONOTONIC_RAW, &ts );
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

static void print_eng( double d )
{
         if( d<1e-6 ) { printf( "~%3.2f n", d*1e9  ); }
    else if( d<1e-3 ) { printf( "~%3.2f u", d*1e6  ); }
    else if( d<1.0  ) { printf( "~%3.2f m", d*1e3  ); }
    else if( d<1e3  ) { printf( "~%3.2f",   d      ); }
    else if( d<1e6  ) { printf( "~%3.2f k", d*1e-3 ); }
    else              { printf( "~%3.2f M", d*1e-6 ); }
}



static void run_fillrate_test( int shader, int mode )
{
    double t0, t1, dt;
    framebuffer fb;
    context ctx;
    int i;

    framebuffer_init( &fb, 1024, 768 );

    /* initialize context */
    memset( &ctx, 0, sizeof(ctx) );
    context_init( &ctx );

    ctx.target = &fb;
    ctx.shader = shader;
    ctx.shade_mode = mode;

    context_set_viewport( &ctx, 0, 0, 1024, 768 );

    ctx.light[0].diffuse = vec4_set( 1.0f, 1.0f, 1.0f, 1.0f );
    ctx.light[0].specular = vec4_set( 1.0f, 1.0f, 1.0f, 1.0f );
    ctx.light[0].enable = 1;
    ctx.light[0].attenuation_constant = 1.0f;

    ctx.material.diffuse = vec4_set( 0.5f, 0.5f, 0.5f, 1.0f );
    ctx.material.specular = vec4_set( 0.5f, 0.5f, 0.5f, 1.0f );
    ctx.material.ambient = vec4_set( 0.0f, 0.0f, 0.0f, 1.0f );
    ctx.material.emission = vec4_set( 0.0f, 0.0f, 0.0f, 1.0f );
    ctx.material.shininess = 127;

    /* drawing loop */
    t0 = get_time( );

    for( i=0; i<100; ++i )
    {
        ia_begin( &ctx );
        ia_color( &ctx, 1.0f, 1.0f, 1.0f, 1.0f );
        ia_normal( &ctx, 0.0f, 0.0f, 1.0f );

        ia_vertex( &ctx, -1.0f,  1.0f, 0.0f, 1.0f );
        ia_vertex( &ctx,  1.0f,  1.0f, 0.0f, 1.0f );
        ia_vertex( &ctx,  1.0f, -1.0f, 0.0f, 1.0f );

        ia_vertex( &ctx, -1.0f,  1.0f, 0.0f, 1.0f );
        ia_vertex( &ctx,  1.0f, -1.0f, 0.0f, 1.0f );
        ia_vertex( &ctx, -1.0f, -1.0f, 0.0f, 1.0f );
        ia_end( &ctx );
    }

    t1 = get_time( );

    /* cleanup */
    framebuffer_cleanup( &fb );
    dt = (t1 - t0) / 100.0;

    /* print result */
    print_eng( (double)(1024*768) / dt );
    puts( " pixels per second" );
}

static void run_vertex_throughput_test( int shader, int mode )
{
    double t0, t1, dt;
    framebuffer fb;
    context ctx;
    int i, j;

    framebuffer_init( &fb, 320, 200 );

    /* initialize context */
    memset( &ctx, 0, sizeof(ctx) );
    context_init( &ctx );

    ctx.flags          = FRONT_CCW|CULL_BACK|CULL_FRONT;
    ctx.target         = &fb;
    ctx.vertex_format  = teapot->format;
    ctx.vertexbuffer   = teapot->vertexbuffer;
    ctx.indexbuffer    = teapot->indexbuffer;
    ctx.shader         = shader;
    ctx.shade_mode     = mode;

    context_set_viewport( &ctx, 0, 0, 320, 200 );

    ctx.light[0].diffuse = vec4_set( 1.0f, 1.0f, 1.0f, 1.0f );
    ctx.light[0].specular = vec4_set( 1.0f, 1.0f, 1.0f, 1.0f );
    ctx.light[0].enable = 1;
    ctx.light[0].attenuation_constant = 1.0f;

    ctx.material.diffuse = vec4_set( 0.5f, 0.5f, 0.5f, 1.0f );
    ctx.material.specular = vec4_set( 0.5f, 0.5f, 0.5f, 1.0f );
    ctx.material.ambient = vec4_set( 0.0f, 0.0f, 0.0f, 1.0f );
    ctx.material.emission = vec4_set( 0.0f, 0.0f, 0.0f, 1.0f );
    ctx.material.shininess = 127;

    /* drawing loop */
    t0 = get_time( );

    for( i=0; i<100; ++i )
    {
        for( j=0; j<20; ++j )
            ia_draw_triangles_indexed(&ctx,teapot->vertices,teapot->indices);
    }

    t1 = get_time( );

    /* cleanup */
    framebuffer_cleanup( &fb );
    dt = (t1 - t0) / 100.0;

    print_eng( (double)(20*teapot->indices) / dt );
    puts( " vertices per second" );
}


int main( void )
{
    teapot = load_3ds( "teapot.3ds" );

    puts( "*********** VERTEX THROUGHPUT TEST ***********" );
    fputs( "BUILT IN UNLIT SHADER: ", stdout );
    run_vertex_throughput_test( SHADER_UNLIT, SHADE_PER_PIXEL );
    fputs( "BUILT IN FLAT SHADER: ", stdout );
    run_vertex_throughput_test( SHADER_GOURAUD, SHADE_FLAT );
    fputs( "BUILT IN GOURAUD SHADER: ", stdout );
    run_vertex_throughput_test( SHADER_GOURAUD, SHADE_PER_PIXEL );
    fputs( "BUILT IN PHONG SHADER: ", stdout );
    run_vertex_throughput_test( SHADER_PHONG, SHADE_PER_PIXEL );

    puts( "*************** FILL RATE TEST ***************" );
    fputs( "BUILT IN UNLIT SHADER: ", stdout );
    run_fillrate_test( SHADER_UNLIT, SHADE_PER_PIXEL );
    fputs( "BUILT IN FLAT SHADER: ", stdout );
    run_fillrate_test( SHADER_GOURAUD, SHADE_FLAT );
    fputs( "BUILT IN GOURAUD SHADER: ", stdout );
    run_fillrate_test( SHADER_GOURAUD, SHADE_PER_PIXEL );
    fputs( "BUILT IN PHONG SHADER: ", stdout );
    run_fillrate_test( SHADER_PHONG, SHADE_PER_PIXEL );

    free( teapot->vertexbuffer );
    free( teapot->indexbuffer );
    free( teapot );
    return 0;
}

