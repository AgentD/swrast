#include "rasterizer.h"
#include "texture.h"
#include "pixel.h"



#ifndef MAX
    #define MAX( a, b, c ) ((a)>(c)?((a)>(b)?(a):(b)) : ((c)>(b)?(c):(b)))
#endif

#ifndef MIN
    #define MIN( a, b, c ) ((a)<(c)?((a)<(b)?(a):(b)) : ((c)<(b)?(c):(b)))
#endif



/****************************************************************************
 *  Rasterizer state control functions                                      *
 ****************************************************************************/

static rs_state rsstate;

void rasterizer_set_state( const rs_state* state )
{
    rsstate = (*state);
}

void rasterizer_get_state( rs_state* state )
{
    (*state) = rsstate;
}

/****************************************************************************
 *  Triangle rasterisation function                                         *
 ****************************************************************************/

void rasterizer_process_triangle( const rs_triangle* t, framebuffer* fb )
{
    int x, y, x0, x1, x2, y0, y1, y2, bl, br, bt, bb, i;
    int f0, f1, f2, f3, f4, f5, f6, f7, f8;
    float a, b, c, f9, f10, f11;
    unsigned char *scan, *ptr;
    rs_fragment A, B, C, v;
    int *dscan, *dptr;

    /* sanity check */
    if( rsstate.cull_cw && rsstate.cull_ccw )
        return;

    if( t->v0.w<=0.0 || t->v1.w<=0.0 || t->v2.w<=0.0 )
        return;

    /* culling */
    f10 = (t->v1.x - t->v0.x)*(t->v2.y - t->v0.y);
    f11 = (t->v1.y - t->v0.y)*(t->v2.x - t->v0.x);

    if( ((f10<=f11) && rsstate.cull_cw) || ((f10>=f11) && rsstate.cull_ccw) )
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

    f9  = 1.0 / (f3*x0 + f6*y0 + f0);
    f10 = 1.0 / (f4*x1 + f7*y1 + f1);
    f11 = 1.0 / (f5*x2 + f8*y2 + f2);

    /* iterate over scanlines in the bounding rectangle */
    scan  = fb->color + (bt*fb->width + bl) * 4;
    dscan = fb->depth + (bt*fb->width + bl);

    for( y=bt; y<=bb; ++y, scan+=fb->width*4, dscan+=fb->width )
    {
        /* iterate over pixels in current scanline */
        for( dptr=dscan, ptr=scan, x=bl; x<=br; ++x, ptr+=4, ++dptr )
        {
            /* determine baricentric coordinates of current pixel */
            a = (f3*x + f6*y + f0) * f9;
            b = (f4*x + f7*y + f1) * f10;
            c = (f5*x + f8*y + f2) * f11;

            /* skip invalid coordinates (outside of triangle) */
            if( a<0.0f || a>1.0f || b<0.0f || b>1.0f || c<0.0f || c>1.0f )
                continue;

            /* interpolate vertex coordinates and perform clipping */
            v.d = A.d*a + B.d*b + C.d*c;
            v.w = 1.0 / (A.w*a + B.w*b + C.w*c);

            if( v.w<=0 || v.d>DEPTH_MAX || v.d<0 )
                continue;

            /* interpolate vertex attributes */
            v.r = (A.r*a + B.r*b + C.r*c) * v.w;
            v.g = (A.g*a + B.g*b + C.g*c) * v.w;
            v.b = (A.b*a + B.b*b + C.b*c) * v.w;
            v.a = (A.a*a + B.a*b + C.a*c) * v.w;

            for( i=0; i<MAX_TEXTURES; ++i )
            {
                v.s[i] = (A.s[i]*a + B.s[i]*b + C.s[i]*c) * v.w;
                v.t[i] = (A.t[i]*a + B.t[i]*b + C.t[i]*c) * v.w;
            }

            /* draw pixel to framebuffer */
            pixel_draw( &v, ptr, dptr );
        }
    }
}

