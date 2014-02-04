#include "texture.h"

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

void texture_sample( texture* t, float x, float y, unsigned char* out )
{
    unsigned char* ptr;
    unsigned int X, Y;

    if( t && out )
    {
        X = x<0.0 ? 0 : x*t->width;
        Y = y<0.0 ? 0 : y*t->height;

        if( X>=t->width )
            X = t->width - 1;

        if( Y>=t->height )
            Y = t->height - 1;

        ptr = t->data + (Y*t->width + X)*4;

        out[0] = ptr[0];
        out[1] = ptr[1];
        out[2] = ptr[2];
        out[3] = ptr[3];
    }
}

