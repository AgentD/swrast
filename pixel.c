#include "compare.h"
#include "texture.h"
#include "pixel.h"



/****************************************************************************
 *  Various implementations of the pixel draw function for differnet        *
 *  configurations (blending enable, etc...)                                *
 ****************************************************************************/

typedef void (* pixel_fun )( const unsigned char* c, unsigned char* );

static void pixel( const unsigned char* c, unsigned char* color )
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
 *  Pixep processor state control functions                                 *
 ****************************************************************************/

static pixel_fun draw_pixel = pixel;
static compare_fun depth_fun;
static pixel_state pp_state;

void pixel_set_state( const pixel_state* s )
{
    if( s )
        pp_state = *s;

    depth_fun = get_compare_function( pp_state.depth_test );
    draw_pixel = pp_state.alpha_blend ? pixel_blend : pixel;
}

void pixel_get_state( pixel_state* s )
{
    if( s )
        *s = pp_state;
}

/****************************************************************************
 *  Pixep processor main draw functions                                     *
 ****************************************************************************/

static void fetch_texture_colors( const rs_vertex* v, unsigned char* color )
{
    unsigned char tex[ 4 ];
    int i;

    for( i=0; i<MAX_TEXTURES; ++i )
    {
        if( pp_state.texture_enable[ i ] )
        {
            texture_sample( pp_state.textures[i], v->s[i], v->t[i], tex );

            color[ RED   ] = (color[ RED   ]*tex[0]) >> 8;
            color[ GREEN ] = (color[ GREEN ]*tex[1]) >> 8;
            color[ BLUE  ] = (color[ BLUE  ]*tex[2]) >> 8;
            color[ ALPHA ] = (color[ ALPHA ]*tex[3]) >> 8;
        }
    }
}

void pixel_draw( const rs_vertex* v, unsigned char* ptr, int* dptr )
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

