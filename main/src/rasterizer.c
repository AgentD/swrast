#include "framebuffer.h"
#include "rasterizer.h"
#include "context.h"
#include "texture.h"
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
    int i;

    vec4_diff( &V->pos, &A->pos, &B->pos );
    vec4_diff( &V->normal, &A->normal, &B->normal );
    vec4_diff( &V->color, &A->color, &B->color );

    vec4_scale( &V->pos, scale );
    vec4_scale( &V->normal, scale );
    vec4_scale( &V->color, scale );

    for( i=0; i<MAX_TEXTURES; ++i )
    {
        vec4_diff( V->texcoord+i, A->texcoord+i, B->texcoord+i );
        vec4_scale( V->texcoord+i, scale );
    }
}

static void scaled_vertex_add( rs_vertex* V, const rs_vertex* A,
                               const rs_vertex* B, float scale )
{
    vec4 v;
    int i;

    vec4_get_scaled( &v, &B->pos, scale );
    vec4_sum( &V->pos, &A->pos, &v );

    vec4_get_scaled( &v, &B->normal, scale );
    vec4_sum( &V->normal, &A->normal, &v );

    vec4_get_scaled( &v, &B->color, scale );
    vec4_sum( &V->color, &A->color, &v );

    for( i=0; i<MAX_TEXTURES; ++i )
    {
        vec4_get_scaled( &v, B->texcoord+i, scale );
        vec4_sum( V->texcoord+i, A->texcoord+i, &v );
    }
}

static void vertex_prepare(rs_vertex* out, const rs_vertex* in, context* ctx)
{
    float d, w = 1.0f / in->pos.w;
    int i;

    /* perspective divide of attributes */
    vec4_get_scaled( &out->normal, &in->normal, w );
    vec4_get_scaled( &out->color, &in->color, w );
    vec4_get_scaled( &out->pos, &in->pos, w );

    for( i=0; i<MAX_TEXTURES; ++i )
        vec4_get_scaled( out->texcoord+i, in->texcoord+i, w );

    /* viewport mapping */
    d = (1.0f - out->pos.z) * 0.5f;

    out->pos.x = (1.0f + out->pos.x) * 0.5f * (float)ctx->viewport.width;
    out->pos.y = (1.0f - out->pos.y) * 0.5f * (float)ctx->viewport.height;
    out->pos.z = d*ctx->depth_far + (1.0f - d)*ctx->depth_near;
    out->pos.w = w;

    out->pos.x += ctx->viewport.x;
    out->pos.y += ctx->viewport.y;
}

/****************************************************************************/

static void draw_scanline( int y, context* ctx, const edge_data* s )
{
    float sub_pixel, w, *z_buffer;
    int x0, x1, i, depthtest[8];
    unsigned char *start, *end;
    vec4 tc, tex, c, old, new;
    scan_line l;

    /* get line start and end */
    x0 = ceil( s->edge[s->left].v.pos.x );
    x1 = ceil( s->edge[s->right].v.pos.x );

    sub_pixel = (float)x0 - s->edge[s->left].v.pos.x;

    /* compuate start values and slopes */
    l.pixelscale = 1.0f/(s->edge[s->right].v.pos.x-s->edge[s->left].v.pos.x);

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
        if( ctx->flags & DEPTH_TEST )
        {
            depthtest[COMPARE_ALWAYS       ] = 1;
            depthtest[COMPARE_NEVER        ] = 0;
            depthtest[COMPARE_LESS         ] = (l.v.pos.z < *z_buffer);
            depthtest[COMPARE_GREATER      ] = (l.v.pos.z > *z_buffer);
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
            (l.v.pos.z > ctx->depth_far || l.v.pos.z < ctx->depth_near) )
        {
            goto skip_fragment;
        }

        w = 1.0f / l.v.pos.w;

        /* interpolate colors */
        vec4_get_scaled( &c, &l.v.color, w );

        /* interpolate texture coordinates, fetch texture colors */
        for( i=0; i<MAX_TEXTURES; ++i )
        {
            if( ctx->texture_enable[ i ] )
            {
                vec4_get_scaled( &tc, l.v.texcoord+i, w );
                texture_sample( ctx->textures[i], &tc, &tex );
                vec4_mul( &c, &tex );
            }
        }

        /* blend onto framebuffer color */
        if( ctx->flags & BLEND_ENABLE )
        {
            vec4_set( &old,start[RED],start[GREEN],start[BLUE],start[ALPHA] );
            vec4_scale( &old, 1.0f/255.0f );
            vec4_mix( &new, &old, &c, c.w );
        }
        else
        {
            new = c;
        }

        vec4_scale( &new, 255.0f );

        if( ctx->flags & WRITE_RED   ) start[RED  ] = new.x;
        if( ctx->flags & WRITE_GREEN ) start[GREEN] = new.y;
        if( ctx->flags & WRITE_BLUE  ) start[BLUE ] = new.z;
        if( ctx->flags & WRITE_ALPHA ) start[ALPHA] = new.w;
        if( ctx->flags & DEPTH_WRITE ) z_buffer[0]  = l.v.pos.z;
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
    y0 = ceil( A->pos.y );
    y1 = ceil( B->pos.y ) - 1;

    advance_line( s, (float)y0 - A->pos.y );

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
    if( A->pos.y > B->pos.y ) { temp_v = A; A = B; B = temp_v; }
    if( B->pos.y > C->pos.y ) { temp_v = B; B = C; C = temp_v; }
    if( A->pos.y > B->pos.y ) { temp_v = A; A = B; B = temp_v; }

    /* calculate y step per line */
    s.linescale[0] = 1.0f / (C->pos.y - A->pos.y);
    s.linescale[1] = 1.0f / (B->pos.y - A->pos.y);
    s.linescale[2] = 1.0f / (C->pos.y - B->pos.y);

    if( s.linescale[0] <= 0.0f )
        return;

    /* check if the major edge is left or right */
    temp[0] = A->pos.x - C->pos.x;
    temp[1] = A->pos.y - C->pos.y;
    temp[2] = B->pos.x - A->pos.x;
    temp[3] = B->pos.y - A->pos.y;

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
            temp[0] = B->pos.y - A->pos.y;
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

    if( v0->pos.w<=0.0f || v1->pos.w<=0.0f || v2->pos.w<=0.0f )
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
    if( (int)A.pos.y>ctx->draw_area.maxy &&
        (int)B.pos.y>ctx->draw_area.maxy &&
        (int)C.pos.y>ctx->draw_area.maxy )
    {
        return;
    }
    if( (int)A.pos.x>ctx->draw_area.maxx &&
        (int)B.pos.x>ctx->draw_area.maxx &&
        (int)C.pos.x>ctx->draw_area.maxx )
    {
        return;
    }
    if( (int)A.pos.y<ctx->draw_area.miny &&
        (int)B.pos.y<ctx->draw_area.miny &&
        (int)C.pos.y<ctx->draw_area.miny )
    {
        return;
    }
    if( (int)A.pos.x<ctx->draw_area.minx &&
        (int)B.pos.x<ctx->draw_area.minx &&
        (int)C.pos.x<ctx->draw_area.minx )
    {
        return;
    }

    /* culling */
    ccw = ((C.pos.x - A.pos.x) * (C.pos.y - B.pos.y) -
           (C.pos.y - A.pos.y) * (C.pos.x - B.pos.x)) < 0.0f;

    cullccw = (ctx->flags & (FRONT_CCW|CULL_FRONT)) == (FRONT_CCW|CULL_FRONT);
    cullccw = cullccw || ((ctx->flags & (FRONT_CCW|CULL_BACK)) == CULL_BACK);

    cullcw = (ctx->flags & (FRONT_CCW|CULL_BACK)) == (FRONT_CCW|CULL_BACK);
    cullcw = cullcw || ((ctx->flags & (FRONT_CCW|CULL_FRONT)) == CULL_FRONT);

    if( (ccw && cullccw) || (!ccw && cullcw) )
        return;

    draw_triangle( &A, &B, &C, ctx );
}

