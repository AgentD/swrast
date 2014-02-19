#include "rasterizer.h"
#include "compare.h"



#ifndef MAX
    #define MAX( a, b, c ) ((a)>(c)?((a)>(b)?(a):(b)) : ((c)>(b)?(c):(b)))
#endif

#ifndef MIN
    #define MIN( a, b, c ) ((a)<(c)?((a)<(b)?(a):(b)) : ((c)<(b)?(c):(b)))
#endif



typedef struct
{
    float w;
    float s[MAX_TEXTURES], t[MAX_TEXTURES];
    unsigned char r, g, b, a;
    int d;
}
rs_vertex;



/****************************************************************************
 *  Various implementations of the pixel draw function for differnet        *
 *  rasterizer configurations (blending, depth test, etc...)                *
 ****************************************************************************/

typedef void (* pixel_fun )( const rs_vertex*, unsigned char*, int* );

static void pixel( const rs_vertex* v, unsigned char* color, int* depth )
{
    color[RED  ] = v->r;
    color[GREEN] = v->g;
    color[BLUE ] = v->b;
    color[ALPHA] = v->a;

    depth[0] = v->d;
}

static void pixel_blend( const rs_vertex* v, unsigned char* color,
                         int* depth )
{
    unsigned int a, ia;
    a  = v->a;
    ia = 0xFF - a;

    color[RED  ] = (color[RED  ]*ia + v->r*a) >> 8;
    color[GREEN] = (color[GREEN]*ia + v->g*a) >> 8;
    color[BLUE ] = (color[BLUE ]*ia + v->b*a) >> 8;
    color[ALPHA] = (color[ALPHA]*ia + v->a*a) >> 8;

    depth[0] = v->d;
}

/****************************************************************************
 *  Rasterizer state control functions                                      *
 ****************************************************************************/

static pixel_fun draw_pixel = pixel;
static rasterizer_state rs_state;
static compare_fun depth_fun;

void rasterizer_set_state( const rasterizer_state* state )
{
    rs_state = (*state);

    depth_fun = get_compare_function( rs_state.depth_test );
    draw_pixel = rs_state.alpha_blend ? pixel_blend : pixel;
}

void rasterizer_get_state( rasterizer_state* state )
{
    (*state) = rs_state;
}

/****************************************************************************
 *  Triangle rasterisation function                                         *
 ****************************************************************************/

void triangle_rasterize( const triangle* t, framebuffer* fb )
{
    float a, b, c, f0, f1, f2, f3, f4, f5, f6, f7, f8;
    int x, y, x0, x1, x2, y0, y1, y2, bl, br, bt, bb, i;
    unsigned char *scan, *ptr;
    unsigned char tex[4];
    rs_vertex A, B, C, v;
    int *dscan, *dptr;

    if( t->v0.w<=0.0 || t->v1.w<=0.0 || t->v2.w<=0.0 )
        return;

    /* prepare triangle vertices */
    A.w = 1.0/t->v0.w;
    A.d = DEPTH_MAX_HALF - (t->v0.z*DEPTH_MAX_HALF*A.w);
    A.r = t->v0.r*A.w;
    A.g = t->v0.g*A.w;
    A.b = t->v0.b*A.w;
    A.a = t->v0.a*A.w;

    B.w = 1.0/t->v1.w;
    B.d = DEPTH_MAX_HALF - (t->v1.z*DEPTH_MAX_HALF*B.w);
    B.r = t->v1.r*B.w;
    B.g = t->v1.g*B.w;
    B.b = t->v1.b*B.w;
    B.a = t->v1.a*B.w;

    C.w = 1.0/t->v2.w;
    C.d = DEPTH_MAX_HALF - (t->v2.z*DEPTH_MAX_HALF*C.w);
    C.r = t->v2.r*C.w;
    C.g = t->v2.g*C.w;
    C.b = t->v2.b*C.w;
    C.a = t->v2.a*C.w;

    for( i=0; i<MAX_TEXTURES; ++i )
    {
        A.s[i]=t->v0.s[i]*A.w; A.t[i]=t->v0.t[i]*A.w;
        B.s[i]=t->v1.s[i]*B.w; B.t[i]=t->v1.t[i]*B.w;
        C.s[i]=t->v2.s[i]*C.w; C.t[i]=t->v2.t[i]*C.w;
    }

    /* convert to raster coordinates */
    x0 = (1.0f + t->v0.x*A.w) * 0.5f * fb->width;
    y0 = (1.0f - t->v0.y*A.w) * 0.5f * fb->height;
    x1 = (1.0f + t->v1.x*B.w) * 0.5f * fb->width;
    y1 = (1.0f - t->v1.y*B.w) * 0.5f * fb->height;
    x2 = (1.0f + t->v2.x*C.w) * 0.5f * fb->width;
    y2 = (1.0f - t->v2.y*C.w) * 0.5f * fb->height;

    /* compute bounding rectangle */
    bl = MIN( x0, x1, x2 );
    br = MAX( x0, x1, x2 );
    bt = MIN( y0, y1, y2 );
    bb = MAX( y0, y1, y2 );

    /* clamp bounding rect to screen */
    bl =  bl<0            ?             0  : bl;
    br = (br>=fb->width)  ? (fb->width -1) : br;
    bt =  bt<0            ?             0  : bt;
    bb = (bb>=fb->height) ? (fb->height-1) : bb;

    /* stop if the bounding rect is invalid or outside screen area */
    if( bl>=br || bt>=bb || bb<0 || bt>=fb->height || br<0 || bl>=fb->width )
        return;

    /* precompute factors for baricentric interpolation */
    f0 = x1*y2 - x2*y1; f3 = y1-y2; f6 = x2-x1;
    f1 = x2*y0 - x0*y2; f4 = y2-y0; f7 = x0-x2;
    f2 = x0*y1 - x1*y0; f5 = y0-y1; f8 = x1-x0;

    a = 1.0f / (f3*x0 + f6*y0 + f0);
    b = 1.0f / (f4*x1 + f7*y1 + f1);
    c = 1.0f / (f5*x2 + f8*y2 + f2);

    f3 *= a; f6 *= a; f0 *= a;
    f4 *= b; f7 *= b; f1 *= b;
    f5 *= c; f8 *= c; f2 *= c;

    /* iterate over scanlines in the bounding rectangle */
    scan  = fb->color + (bt*fb->width + bl) * 4;
    dscan = fb->depth + (bt*fb->width + bl);

    for( y=bt; y<=bb; ++y, scan+=fb->width*4, dscan+=fb->width )
    {
        /* iterate over pixels in current scanline */
        for( dptr=dscan, ptr=scan, x=bl; x<=br; ++x, ptr+=4, ++dptr )
        {
            /* determine baricentric coordinates of current pixel */
            a = f3*x + f6*y + f0;
            b = f4*x + f7*y + f1;
            c = f5*x + f8*y + f2;

            /* skip invalid coordinates (outside of triangle) */
            if( a<0.0f || a>1.0f || b<0.0f || b>1.0f || c<0.0f || c>1.0f )
                continue;

            /* interpolate vertex coordinates and perform clipping */
            v.d = A.d*a + B.d*b + C.d*c;
            v.w = 1.0 / (A.w*a + B.w*b + C.w*c);

            if( v.w<=0 || v.d>DEPTH_MAX || v.d<0 )
                continue;

            /* depth test */
            if( !depth_fun( v.d, dptr[0] ) )
                continue;

            /* interpolate vertex attributes */
            v.r = (A.r*a + B.r*b + C.r*c) * v.w;
            v.g = (A.g*a + B.g*b + C.g*c) * v.w;
            v.b = (A.b*a + B.b*b + C.b*c) * v.w;
            v.a = (A.a*a + B.a*b + C.a*c) * v.w;

            /* apply texture values */
            for( i=0; i<MAX_TEXTURES; ++i )
            {
                if( rs_state.texture_enable[ i ] )
                {
                    v.s[i] = (A.s[i]*a + B.s[i]*b + C.s[i]*c) * v.w;
                    v.t[i] = (A.t[i]*a + B.t[i]*b + C.t[i]*c) * v.w;

                    texture_sample(rs_state.textures[i], v.s[i], v.t[i], tex);

                    v.r = (v.r*tex[0]) >> 8;
                    v.g = (v.g*tex[1]) >> 8;
                    v.b = (v.b*tex[2]) >> 8;
                    v.a = (v.a*tex[3]) >> 8;
                }
            }

            /* draw pixel to framebuffer */
            draw_pixel( &v, ptr, dptr );
        }
    }
}

