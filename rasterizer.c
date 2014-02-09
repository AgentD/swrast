#include "rasterizer.h"



#ifndef MAX
    #define MAX( a, b, c ) ((a)>(c)?((a)>(b)?(a):(b)) : ((c)>(b)?(c):(b)))
#endif

#ifndef MIN
    #define MIN( a, b, c ) ((a)<(c)?((a)<(b)?(a):(b)) : ((c)<(b)?(c):(b)))
#endif



/****************************************************************************
 *  Various implementations of the pixel draw function for differnet        *
 *  rasterizer configurations (blending, depth test, etc...)                *
 ****************************************************************************/

typedef void (* pixel_fun )( vertex*, unsigned char*, float* );

static void pixel( vertex* v, unsigned char* color, float* depth )
{
    color[RED  ] = v->r * 255.0;
    color[GREEN] = v->g * 255.0;
    color[BLUE ] = v->b * 255.0;
    color[ALPHA] = v->a * 255.0;

    depth[0] = (1.0f - v->z) * 0.5f;
}

static void pixel_depth( vertex* v, unsigned char* color, float* depth )
{
    v->z = (1.0f - v->z) * 0.5f;

    if( v->z < depth[0] )
    {
        color[RED  ] = v->r * 255.0;
        color[GREEN] = v->g * 255.0;
        color[BLUE ] = v->b * 255.0;
        color[ALPHA] = v->a * 255.0;

        depth[0] = v->z;
    }
}

static void pixel_blend( vertex* v, unsigned char* color, float* depth )
{
    unsigned int r, g, b, a, ia;
    a  = v->a * 255.0;
    ia = 0xFF - a;
    r  = v->r * a;
    g  = v->g * a;
    b  = v->b * a;
    a  = v->a * a;

    color[RED  ] = ((color[RED  ]*ia) >> 8) + r;
    color[GREEN] = ((color[GREEN]*ia) >> 8) + g;
    color[BLUE ] = ((color[BLUE ]*ia) >> 8) + b;
    color[ALPHA] = ((color[ALPHA]*ia) >> 8) + a;

    depth[0] = (1.0f - v->z) * 0.5f;
}

static void pixel_depth_blend( vertex* v, unsigned char* color, float* depth )
{
    unsigned int r, g, b, a, ia;
    v->z = (1.0f - v->z) * 0.5f;

    if( v->z < depth[0] )
    {
        a  = v->a * 255.0;
        ia = 0xFF - a;
        r  = v->r * a;
        g  = v->g * a;
        b  = v->b * a;
        a  = v->a * a;

        color[RED  ] = ((color[RED  ]*ia) >> 8) + r;
        color[GREEN] = ((color[GREEN]*ia) >> 8) + g;
        color[BLUE ] = ((color[BLUE ]*ia) >> 8) + b;
        color[ALPHA] = ((color[ALPHA]*ia) >> 8) + a;

        depth[0] = v->z;
    }
}

/****************************************************************************
 *  Rasterizer state control functions                                      *
 ****************************************************************************/

static pixel_fun draw_pixel = pixel;
static rasterizer_state rs_state;


void rasterizer_set_state( const rasterizer_state* state )
{
    rs_state = (*state);

    if( rs_state.alpha_blend )
    {
        draw_pixel = rs_state.depth_test ? pixel_depth_blend : pixel_blend;
    }
    else
    {
        draw_pixel = rs_state.depth_test ? pixel_depth : pixel;
    }
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
    float a, b, c, f0, f1, f2, f3, f4, f5, f6, f7, f8, *dscan, *dptr;
    int x, y, x0, x1, x2, y0, y1, y2, bl, br, bt, bb, i;
    unsigned char *scan, *ptr;
    unsigned char tex[4];
    vertex A, B, C, v;

    if( t->v0.w<=0.0 || t->v1.w<=0.0 || t->v2.w<=0.0 )
        return;

    /* prepare triangle vertices */
    A.w = 1.0/t->v0.w;
    B.w = 1.0/t->v1.w;
    C.w = 1.0/t->v2.w;

    A.x=t->v0.x*A.w; A.y=t->v0.y*A.w; A.z=t->v0.z*A.w;
    A.r=t->v0.r*A.w; A.g=t->v0.g*A.w; A.b=t->v0.b*A.w; A.a=t->v0.a*A.w;

    B.x=t->v1.x*B.w; B.y=t->v1.y*B.w; B.z=t->v1.z*B.w;
    B.r=t->v1.r*B.w; B.g=t->v1.g*B.w; B.b=t->v1.b*B.w; B.a=t->v1.a*B.w;

    C.x=t->v2.x*C.w; C.y=t->v2.y*C.w; C.z=t->v2.z*C.w;
    C.r=t->v2.r*C.w; C.g=t->v2.g*C.w; C.b=t->v2.b*C.w; C.a=t->v2.a*C.w;

    for( i=0; i<MAX_TEXTURES; ++i )
    {
        A.s[i]=t->v0.s[i]*A.w; A.t[i]=t->v0.t[i]*A.w;
        B.s[i]=t->v1.s[i]*B.w; B.t[i]=t->v1.t[i]*B.w;
        C.s[i]=t->v2.s[i]*C.w; C.t[i]=t->v2.t[i]*C.w;
    }

    /* convert to raster coordinates */
    x0 = (1.0f + A.x) * 0.5f * fb->width;
    y0 = (1.0f - A.y) * 0.5f * fb->height;
    x1 = (1.0f + B.x) * 0.5f * fb->width;
    y1 = (1.0f - B.y) * 0.5f * fb->height;
    x2 = (1.0f + C.x) * 0.5f * fb->width;
    y2 = (1.0f - C.y) * 0.5f * fb->height;

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
            v.x = A.x*a + B.x*b + C.x*c;
            v.y = A.y*a + B.y*b + C.y*c;
            v.z = A.z*a + B.z*b + C.z*c;
            v.w = 1.0 / (A.w*a + B.w*b + C.w*c);

            if( v.w<=0 || v.x>v.w || v.y>v.w || v.z>v.w )
                continue;

            if( v.x<-v.w || v.y<-v.w || v.z<-v.w )
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

                    v.r *= (tex[0]/255.0);
                    v.g *= (tex[1]/255.0);
                    v.b *= (tex[2]/255.0);
                    v.a *= (tex[3]/255.0);
                }
            }

            /* draw pixel to framebuffer */
            draw_pixel( &v, ptr, dptr );
        }
    }
}

