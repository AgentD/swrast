#include "texture.h"
#include "config.h"
#include "vector.h"

#include <stdlib.h>



texture* texture_create( unsigned int width, unsigned int height )
{
    texture* t = malloc( sizeof(texture) );

    t->data = malloc( width*height*4 );

    if( !t->data )
    {
        free( t );
        return NULL;
    }

    t->width  = width;
    t->height = height;
    return t;
}

void texture_destroy( texture* t )
{
    if( t )
    {
        free( t->data );
        free( t );
    }
}

void texture_sample( texture* t, const vec4* tc, vec4* out )
{
    unsigned char* ptr;
    unsigned int X, Y;

    if( t && tc && out )
    {
        X = tc->x<0.0f ? 0.0f : tc->x*t->width;
        Y = tc->y<0.0f ? 0.0f : tc->y*t->height;

        if( X>=t->width )
            X = t->width - 1;

        if( Y>=t->height )
            Y = t->height - 1;

        ptr = t->data + (Y*t->width + X)*4;

        out->x = (float)ptr[0] / 255.0f;
        out->y = (float)ptr[1] / 255.0f;
        out->z = (float)ptr[2] / 255.0f;
        out->w = (float)ptr[3] / 255.0f;
    }
}

