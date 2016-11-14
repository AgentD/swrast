#include "framebuffer.h"
#include "rasterizer.h"
#include "context.h"
#include "texture.h"
#include "shader.h"
#include <math.h>



typedef struct
{
    int left;                       /* index of left edge */
    int right;                      /* index of right edge */

    float linescale[3];             /* 1 / dy */

    struct
    {
        rs_vertex v;                /* current vertex */
        rs_vertex dvdy;             /* difference per line */
    }
    edge[2];
}
edge_data;

typedef struct
{
    float pixelscale;           /* 1.0/dx */

    rs_vertex v;                /* current vertex */
    rs_vertex dvdx;             /* difference per pixel */
}
scan_line;



static void scaled_vertex_diff( rs_vertex* V, const rs_vertex* A,
                                const rs_vertex* B, float scale )
{
    int i, j;

    V->used = A->used & B->used;

    for( i=0, j=0x01; i<ATTRIB_COUNT; ++i, j<<=1 )
    {
        if( V->used & j )
        {
            V->attribs[i] = vec4_scale( vec4_sub(A->attribs[i],B->attribs[i]),
                                        scale );
        }
    }
}

static void scaled_vertex_add( rs_vertex* V, const rs_vertex* A,
                               const rs_vertex* B, float scale )
{
    int i, j;

    V->used = A->used & B->used;

    for( i=0, j=0x01; i<ATTRIB_COUNT; ++i, j<<=1 )
    {
        if( V->used & j )
        {
            V->attribs[i] = vec4_add( A->attribs[i],
                                      vec4_scale(B->attribs[i], scale) );
        }
    }
}

static void vertex_prepare(rs_vertex* out, const rs_vertex* in, context* ctx)
{
    float d, w = 1.0f / in->attribs[ATTRIB_POS].w;
    int i, j;
    vec4 v;

    /* perspective divide of attributes */
    for( i=0, j=0x01; i<ATTRIB_COUNT; ++i, j<<=1 )
    {
        if( in->used & j )
            out->attribs[i] = vec4_scale( in->attribs[i], w );
    }

    /* viewport mapping */
    v = out->attribs[ATTRIB_POS];
    d = (1.0f - v.z) * 0.5f;

    v.x = (1.0f + v.x) * 0.5f * (float)ctx->viewport.width  + ctx->viewport.x;
    v.y = (1.0f - v.y) * 0.5f * (float)ctx->viewport.height + ctx->viewport.y;
    v.z = d*ctx->depth_far + (1.0f - d)*ctx->depth_near;
    v.w = w;

    out->attribs[ATTRIB_POS] = v;
    out->used = in->used;
}

/****************************************************************************/

static void write_fragment( context* ctx,
                            const vec4 frag_color, float frag_depth,
                            unsigned char* color_buffer, float* depth_buffer )
{
    vec4 old, new;

    if( ctx->flags & BLEND_ENABLE )
    {
        old = vec4_set( color_buffer[RED], color_buffer[GREEN],
                        color_buffer[BLUE], color_buffer[ALPHA] );
        old = vec4_scale( old, 1.0f/255.0f );
        new = vec4_mix( old, frag_color, frag_color.w );
    }
    else
    {
        new = frag_color;
    }

    new = vec4_scale( new, 255.0f );

    if( ctx->flags & WRITE_RED   ) color_buffer[RED  ] = new.x;
    if( ctx->flags & WRITE_GREEN ) color_buffer[GREEN] = new.y;
    if( ctx->flags & WRITE_BLUE  ) color_buffer[BLUE ] = new.z;
    if( ctx->flags & WRITE_ALPHA ) color_buffer[ALPHA] = new.w;
    if( ctx->flags & DEPTH_WRITE ) depth_buffer[0    ] = frag_depth;
}

static void draw_scanline( int y, context* ctx, const edge_data* s )
{
    float sub_pixel, z, w, *z_buffer;
    int x0, x1, i, j, depthtest[8];
    unsigned char *start, *end;
    rs_vertex frag;
    scan_line l;
    vec4 c;

    /* get line start and end */
    x0 = ceil( s->edge[s->left].v.attribs[ATTRIB_POS].x );
    x1 = ceil( s->edge[s->right].v.attribs[ATTRIB_POS].x );

    sub_pixel = (float)x0 - s->edge[s->left].v.attribs[ATTRIB_POS].x;

    /* compuate start values and slopes */
    l.pixelscale = 1.0f/(s->edge[s->right].v.attribs[ATTRIB_POS].x -
                         s->edge[s->left].v.attribs[ATTRIB_POS].x);

    scaled_vertex_diff( &l.dvdx, &s->edge[s->right].v, &s->edge[s->left].v,
                        l.pixelscale );

    scaled_vertex_add( &l.v, &s->edge[s->left].v, &l.dvdx, sub_pixel );

    /* get pointers to target scan line */
    if( x0<ctx->draw_area.minx )
    {
        scaled_vertex_add( &l.v, &l.v, &l.dvdx, ctx->draw_area.minx - x0 );
        x0 = ctx->draw_area.minx;
    }

    if( x1>ctx->draw_area.maxx )
        x1 = ctx->draw_area.maxx;

    if( x1<x0 || x0>ctx->draw_area.maxx || x1<ctx->draw_area.minx )
        return;

    z_buffer = ctx->target->depth + (y*ctx->target->width + x0);
    start = ctx->target->color + (y*ctx->target->width + x0)*4;
    end = ctx->target->color + (y*ctx->target->width + x1)*4;

    /* for each fragment */
    for( ; start!=end && x0<=ctx->draw_area.maxx; start+=4, ++z_buffer, ++x0 )
    {
        /* depth test */
        z = l.v.attribs[ATTRIB_POS].z;

        if( ctx->flags & DEPTH_TEST )
        {
            depthtest[COMPARE_ALWAYS       ] = 1;
            depthtest[COMPARE_NEVER        ] = 0;
            depthtest[COMPARE_LESS         ] = (z < *z_buffer);
            depthtest[COMPARE_GREATER      ] = (z > *z_buffer);
            depthtest[COMPARE_NOT_EQUAL    ] = depthtest[COMPARE_LESS] |
                                               depthtest[COMPARE_GREATER];
            depthtest[COMPARE_EQUAL        ] = !depthtest[COMPARE_NOT_EQUAL];
            depthtest[COMPARE_LESS_EQUAL   ] = depthtest[COMPARE_EQUAL] |
                                               depthtest[COMPARE_LESS];
            depthtest[COMPARE_GREATER_EQUAL] = depthtest[COMPARE_EQUAL] |
                                               depthtest[COMPARE_GREATER];

            if( !depthtest[ctx->depth_test] )
                goto skip_fragment;
        }
        if( (ctx->flags & DEPTH_CLIP) &&
            (z > ctx->depth_far || z < ctx->depth_near) )
        {
            goto skip_fragment;
        }

        /* calculate interpolated attributes */
        w = 1.0f / l.v.attribs[ATTRIB_POS].w;
        frag.used = l.v.used;

        for( i=0, j=0x01; i<ATTRIB_COUNT; ++i, j<<=1 )
        {
            if( frag.used & j )
                frag.attribs[i] = vec4_scale( l.v.attribs[i], w );
        }

        /* */
        c = shader_process_fragment( ctx, &frag );

        write_fragment( ctx, c, z, start, z_buffer );
    skip_fragment:
        scaled_vertex_add( &l.v, &l.v, &l.dvdx, 1.0f );
    }
}

static void advance_line( edge_data* s, float scale )
{
    scaled_vertex_add(&s->edge[0].v, &s->edge[0].v, &s->edge[0].dvdy, scale);
    scaled_vertex_add(&s->edge[1].v, &s->edge[1].v, &s->edge[1].dvdy, scale);
}

static void draw_half_triangle( edge_data* s, rs_vertex* A, rs_vertex* B,
                                context* ctx )
{
    int y0, y1;

    /* apply top-left fill convention */
    y0 = ceil( A->attribs[ATTRIB_POS].y );
    y1 = ceil( B->attribs[ATTRIB_POS].y ) - 1;

    advance_line( s, (float)y0 - A->attribs[ATTRIB_POS].y );

    /* draw scanlines */
    if( y0 < ctx->draw_area.miny )
    {
        advance_line( s, ctx->draw_area.miny - y0 );
        y0 = ctx->draw_area.miny;
    }

    for( ; y0<=y1 && y0<=ctx->draw_area.maxy; ++y0 )
    {
        draw_scanline( y0, ctx, s );
        advance_line( s, 1.0f );
    }
}

static void draw_triangle( rs_vertex* A, rs_vertex* B,
                           rs_vertex* C, context* ctx )
{
    rs_vertex* temp_v;
    float temp[4];
    edge_data s;

    /* sort on Y axis */
    if( A->attribs[ATTRIB_POS].y > B->attribs[ATTRIB_POS].y )
    {
        temp_v = A; A = B; B = temp_v;
    }
    if( B->attribs[ATTRIB_POS].y > C->attribs[ATTRIB_POS].y )
    {
        temp_v = B; B = C; C = temp_v;
    }
    if( A->attribs[ATTRIB_POS].y > B->attribs[ATTRIB_POS].y )
    {
        temp_v = A; A = B; B = temp_v;
    }

    /* calculate y step per line */
    s.linescale[0] = 1.0f/(C->attribs[ATTRIB_POS].y-A->attribs[ATTRIB_POS].y);
    s.linescale[1] = 1.0f/(B->attribs[ATTRIB_POS].y-A->attribs[ATTRIB_POS].y);
    s.linescale[2] = 1.0f/(C->attribs[ATTRIB_POS].y-B->attribs[ATTRIB_POS].y);

    if( s.linescale[0] <= 0.0f )
        return;

    /* check if the major edge is left or right */
    temp[0] = A->attribs[ATTRIB_POS].x - C->attribs[ATTRIB_POS].x;
    temp[1] = A->attribs[ATTRIB_POS].y - C->attribs[ATTRIB_POS].y;
    temp[2] = B->attribs[ATTRIB_POS].x - A->attribs[ATTRIB_POS].x;
    temp[3] = B->attribs[ATTRIB_POS].y - A->attribs[ATTRIB_POS].y;

    s.left = (temp[0]*temp[3] - temp[1]*temp[2]) > 0.0f ? 0 : 1;
    s.right = !s.left;

    /* calculate slopes for major edge */
    s.edge[0].v = *A;
    scaled_vertex_diff( &s.edge[0].dvdy, C, A, s.linescale[0] );

    /* rasterize upper sub-triangle */
    if( s.linescale[1] > 0.0f )
    {
        /* calculate slopes for minor edge */
        s.edge[1].v = *A;
        scaled_vertex_diff( &s.edge[1].dvdy, B, A, s.linescale[1] );

        /* rasterize the edge scanlines */
        draw_half_triangle( &s, A, B, ctx );
    }

    /* rasterize lower sub-triangle */
    if( s.linescale[2] > 0.0f )
    {
        /* advance to center */
        if( s.linescale[1] > 0.0f )
        {
            temp[0] = B->attribs[ATTRIB_POS].y - A->attribs[ATTRIB_POS].y;
            scaled_vertex_add( &s.edge[0].v, A, &s.edge[0].dvdy, temp[0] );
        }

        /* calculate slopes for bottom edge */
        s.edge[1].v = *B;
        scaled_vertex_diff( &s.edge[1].dvdy, C, B, s.linescale[2] );

        draw_half_triangle( &s, B, C, ctx );
    }
}

/****************************************************************************/

void rasterizer_process_triangle( context* ctx, const rs_vertex* v0,
                                  const rs_vertex* v1, const rs_vertex* v2 )
{
    int ccw, cullccw, cullcw;
    rs_vertex A, B, C;

    /* sanity check */
    if( (ctx->flags & CULL_FRONT) && (ctx->flags & CULL_BACK) )
        return;

    if( (ctx->flags & DEPTH_TEST) && ctx->depth_test==COMPARE_NEVER )
        return;

    if( v0->attribs[ATTRIB_POS].w<=0.0f || v1->attribs[ATTRIB_POS].w<=0.0f ||
        v2->attribs[ATTRIB_POS].w<=0.0f )
        return;

    if( ctx->draw_area.minx>=ctx->draw_area.maxx ||
        ctx->draw_area.miny>=ctx->draw_area.maxy )
    {
        return;
    }

    /* prepare vertices */
    vertex_prepare( &A, v0, ctx );
    vertex_prepare( &B, v1, ctx );
    vertex_prepare( &C, v2, ctx );

    /* clipping */
    if( (int)A.attribs[ATTRIB_POS].y>ctx->draw_area.maxy &&
        (int)B.attribs[ATTRIB_POS].y>ctx->draw_area.maxy &&
        (int)C.attribs[ATTRIB_POS].y>ctx->draw_area.maxy )
    {
        return;
    }
    if( (int)A.attribs[ATTRIB_POS].x>ctx->draw_area.maxx &&
        (int)B.attribs[ATTRIB_POS].x>ctx->draw_area.maxx &&
        (int)C.attribs[ATTRIB_POS].x>ctx->draw_area.maxx )
    {
        return;
    }
    if( (int)A.attribs[ATTRIB_POS].y<ctx->draw_area.miny &&
        (int)B.attribs[ATTRIB_POS].y<ctx->draw_area.miny &&
        (int)C.attribs[ATTRIB_POS].y<ctx->draw_area.miny )
    {
        return;
    }
    if( (int)A.attribs[ATTRIB_POS].x<ctx->draw_area.minx &&
        (int)B.attribs[ATTRIB_POS].x<ctx->draw_area.minx &&
        (int)C.attribs[ATTRIB_POS].x<ctx->draw_area.minx )
    {
        return;
    }

    /* culling */
    ccw = ((C.attribs[ATTRIB_POS].x - A.attribs[ATTRIB_POS].x) *
           (C.attribs[ATTRIB_POS].y - B.attribs[ATTRIB_POS].y) -
           (C.attribs[ATTRIB_POS].y - A.attribs[ATTRIB_POS].y) *
           (C.attribs[ATTRIB_POS].x - B.attribs[ATTRIB_POS].x)) < 0.0f;

    cullccw = (ctx->flags & (FRONT_CCW|CULL_FRONT)) == (FRONT_CCW|CULL_FRONT);
    cullccw = cullccw || ((ctx->flags & (FRONT_CCW|CULL_BACK)) == CULL_BACK);

    cullcw = (ctx->flags & (FRONT_CCW|CULL_BACK)) == (FRONT_CCW|CULL_BACK);
    cullcw = cullcw || ((ctx->flags & (FRONT_CCW|CULL_FRONT)) == CULL_FRONT);

    if( (ccw && cullccw) || (!ccw && cullcw) )
        return;

    draw_triangle( &A, &B, &C, ctx );
}

