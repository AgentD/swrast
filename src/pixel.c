#include "texture.h"
#include "pixel.h"

#include <stddef.h>



typedef int (* compare_fun )( int a, int b );



static int compare_always( int a, int b )
{
    (void)a; (void)b;
    return 1;
}

static int compare_never( int a, int b )
{
    (void)a; (void)b;
    return 0;
}

static int compare_equal( int a, int b )
{
    return a==b;
}

static int compare_not_equal( int a, int b )
{
    return a!=b;
}

static int compare_less( int a, int b )
{
    return a<b;
}

static int compare_less_equal( int a, int b )
{
    return a<=b;
}

static int compare_greater( int a, int b )
{
    return a>b;
}

static int compare_greater_equal( int a, int b )
{
    return a>=b;
}

static compare_fun get_compare_function( int comparison )
{
    switch( comparison )
    {
    case COMPARE_ALWAYS:        return compare_always;
    case COMPARE_NEVER:         return compare_never;
    case COMPARE_EQUAL:         return compare_equal;
    case COMPARE_NOT_EQUAL:     return compare_not_equal;
    case COMPARE_LESS:          return compare_less;
    case COMPARE_LESS_EQUAL:    return compare_less_equal;
    case COMPARE_GREATER:       return compare_greater;
    case COMPARE_GREATER_EQUAL: return compare_greater_equal;
    }

    return NULL;
}

/****************************************************************************
 *  Various implementations of the pixel draw function for differnet        *
 *  configurations (blending enable, etc...)                                *
 ****************************************************************************/

typedef void (* pixel_fun )( const unsigned char* c, unsigned char* );

static void pixel_noblend( const unsigned char* c, unsigned char* color )
{
    color[RED  ] = c[RED  ];
    color[GREEN] = c[GREEN];
    color[BLUE ] = c[BLUE ];
    color[ALPHA] = c[ALPHA];
}

static void pixel_blend( const unsigned char* c, unsigned char* color )
{
    unsigned int a, ia;
    a = c[ALPHA];
    ia = 0xFF - a;

    color[RED  ] = (color[RED  ]*ia + c[RED  ]*a) >> 8;
    color[GREEN] = (color[GREEN]*ia + c[GREEN]*a) >> 8;
    color[BLUE ] = (color[BLUE ]*ia + c[BLUE ]*a) >> 8;
    color[ALPHA] = (color[ALPHA]*ia + (a<<8)    ) >> 8;

}

/****************************************************************************
 *  Pixel processor state control functions                                 *
 ****************************************************************************/

static pixel_fun draw_pixel = pixel_noblend;
static compare_fun depth_fun = compare_always;
static pixel_state pp_state =
{
    COMPARE_ALWAYS,
    0,
    { 0 },
    { 0 }
};



void pixel_set_state( const pixel_state* s )
{
    if( s )
        pp_state = *s;

    depth_fun = get_compare_function( pp_state.depth_test );
    draw_pixel = pp_state.alpha_blend ? pixel_blend : pixel_noblend;
}

void pixel_get_state( pixel_state* s )
{
    if( s )
        *s = pp_state;
}

/****************************************************************************
 *  Pixel processor main draw functions                                     *
 ****************************************************************************/

static void fetch_texture_colors( const rs_fragment* v, unsigned char* color )
{
    unsigned char tex[ 4 ];
    int i;

    for( i=0; i<MAX_TEXTURES; ++i )
    {
        if( pp_state.texture_enable[ i ] )
        {
            texture_sample( pp_state.textures[i], v->s[i], v->t[i], tex );

            color[ RED   ] = (color[ RED   ]*tex[ RED   ]) >> 8;
            color[ GREEN ] = (color[ GREEN ]*tex[ GREEN ]) >> 8;
            color[ BLUE  ] = (color[ BLUE  ]*tex[ BLUE  ]) >> 8;
            color[ ALPHA ] = (color[ ALPHA ]*tex[ ALPHA ]) >> 8;
        }
    }
}

void pixel_draw( const rs_fragment* v, unsigned char* ptr, int* dptr )
{
    unsigned char c[4];

    /* depth test */
    if( !depth_fun( v->d, *dptr ) )
        return;

    c[RED  ] = v->r;
    c[GREEN] = v->g;
    c[BLUE ] = v->b;
    c[ALPHA] = v->a;
    fetch_texture_colors( v, c );

    *dptr = v->d;
    draw_pixel( c, ptr );    
}

