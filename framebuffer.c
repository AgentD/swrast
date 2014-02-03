#include "framebuffer.h"

#include <stdlib.h>



framebuffer* framebuffer_create( unsigned int width, unsigned int height )
{
    framebuffer* fb = malloc( sizeof(framebuffer) );

    if( !fb )
        return NULL;

    fb->width  = width;
    fb->height = height;

    /* try to allocate the color buffer */
    fb->color = malloc( width*height*4 );

    if( !fb->color )
    {
        free( fb );
        return NULL;
    }

    /* try to allocate the depth buffer */
    fb->depth = malloc( width*height*sizeof(float) );

    if( !fb->depth )
    {
        free( fb->color );
        free( fb );
        return NULL;
    }

    return fb;
}

void framebuffer_destroy( framebuffer* fb )
{
    if( fb )
    {
        free( fb->depth );
        free( fb->color );
        free( fb );
    }
}

void framebuffer_clear( framebuffer* fb, unsigned char r, unsigned char g,
                                         unsigned char b, unsigned char a )
{
    unsigned int i, count;
    unsigned char* ptr;

    if( fb )
    {
        count = fb->height*fb->width;

        for( ptr=fb->color, i=0; i<count; ++i, ptr+=4 )
        {
            ptr[ RED   ] = r;
            ptr[ GREEN ] = g;
            ptr[ BLUE  ] = b;
            ptr[ ALPHA ] = a;
        }
    }
}

void framebuffer_clear_depth( framebuffer* fb, float value )
{
    unsigned int i, count;
    float* ptr;

    if( fb )
    {
        count = fb->height*fb->width;

        for( ptr=fb->depth, i=0; i<count; ++i, ++ptr )
        {
            *ptr = value;
        }
    }
}

