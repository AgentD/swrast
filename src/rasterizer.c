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



static void scaled_vertex_diff( rs_vertex* v, const rs_vertex* A,
                                const rs_vertex* B, float scale )
{
    int i;

    v->x = (A->x - B->x) * scale;
    v->w = (A->w - B->w) * scale;
    v->z = (A->z - B->z) * scale;

    v->r = (A->r - B->r) * scale;
    v->g = (A->g - B->g) * scale;
    v->b = (A->b - B->b) * scale;
    v->a = (A->a - B->a) * scale;

    for( i=0; i<MAX_TEXTURES; ++i )
    {
        v->s[i] = (A->s[i] - B->s[i]) * scale;
        v->t[i] = (A->t[i] - B->t[i]) * scale;
    }
}

static void scaled_vertex_add( rs_vertex* v, const rs_vertex* A,
                               const rs_vertex* B, float scale )
{
    int i;

    v->x = A->x + B->x * scale;
    v->z = A->z + B->z * scale;
    v->w = A->w + B->w * scale;

    v->r = A->r + B->r * scale;
    v->g = A->g + B->g * scale;
    v->b = A->b + B->b * scale;
    v->a = A->a + B->a * scale;

    for( i=0; i<MAX_TEXTURES; ++i )
    {
        v->s[i] = A->s[i] + B->s[i] * scale;
        v->t[i] = A->t[i] + B->t[i] * scale;
    }
}

static void vertex_prepare(rs_vertex* out, const rs_vertex* in, context* ctx)
{
    float d;
    int i;

    /* perspective divide and viewport mapping */
    out->w = 1.0/in->w;
    out->x = (1.0f + in->x*out->w) * 0.5f * (float)ctx->viewport.width;
    out->y = (1.0f - in->y*out->w) * 0.5f * (float)ctx->viewport.height;

    d = (1.0f - in->z*out->w) * 0.5f;
    out->z = d*ctx->depth_far + (1.0f - d)*ctx->depth_near;

    out->x += ctx->viewport.x;
    out->y += ctx->viewport.y;

    /* perspective divide of attributes */
    out->r = in->r * out->w;
    out->g = in->g * out->w;
    out->b = in->b * out->w;
    out->a = in->a * out->w;

    for( i=0; i<MAX_TEXTURES; ++i )
    {
        out->s[i] = in->s[i] * out->w;
        out->t[i] = in->t[i] * out->w;
    }
}

/****************************************************************************/

static void draw_scanline( int y, context* ctx, const edge_data* s )
{
    float sub_pixel, w, *z_buffer, tex[4], c[4];
    int x0, x1, i, j, depthtest[8];
    unsigned char *start, *end;
    scan_line l;

    /* get line start and end */
    x0 = ceil( s->edge[s->left].v.x );
    x1 = ceil( s->edge[s->right].v.x );

    sub_pixel = (float)x0 - s->edge[s->left].v.x;

    /* compuate start values and slopes */
    l.pixelscale = 1.0f / (s->edge[s->right].v.x - s->edge[s->left].v.x);

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
        if( ctx->depth_test!=COMPARE_ALWAYS )
        {
            depthtest[COMPARE_LESS         ] = (l.v.z < *z_buffer);
            depthtest[COMPARE_GREATER      ] = (l.v.z > *z_buffer);
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
        if( ctx->depth_clip &&
            (l.v.z > ctx->depth_far || l.v.z < ctx->depth_near) )
        {
            goto skip_fragment;
        }

        w = 1.0f / l.v.w;

        /* interpolate colors */
        c[RED  ] = l.v.r * w;
        c[GREEN] = l.v.g * w;
        c[BLUE ] = l.v.b * w;
        c[ALPHA] = l.v.a * w;

        /* interpolate texture coordinates, fetch texture colors */
        for( i=0; i<MAX_TEXTURES; ++i )
        {
            if( ctx->texture_enable[ i ] )
            {
                texture_sample(ctx->textures[i], l.v.s[i]*w, l.v.t[i]*w, tex);

                for( j=0; j<4; ++j )
                    c[j] *= tex[j];
            }
        }

        /* blend onto framebuffer color */
        if( ctx->alpha_blend )
        {
            float a = c[ALPHA], ia = 1.0f - a;

            c[RED  ] = start[RED  ]*ia + c[RED  ]*a*255.0f;
            c[GREEN] = start[GREEN]*ia + c[GREEN]*a*255.0f;
            c[BLUE ] = start[BLUE ]*ia + c[BLUE ]*a*255.0f;
            c[ALPHA] = start[ALPHA]*ia +          a*255.0f;

            for( i=0; i<4; ++i )
                start[i] = (unsigned char)c[i];
        }
        else
        {
            for( i=0; i<4; ++i )
                start[i] = (unsigned char)(c[i]*255.0f);
        }

        if( ctx->depth_write )
            *z_buffer = l.v.z;
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
    y0 = ceil( A->y );
    y1 = ceil( B->y ) - 1;

    advance_line( s, (float)y0 - A->y );

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
    if( A->y > B->y ) { temp_v = A; A = B; B = temp_v; }
    if( B->y > C->y ) { temp_v = B; B = C; C = temp_v; }
    if( A->y > B->y ) { temp_v = A; A = B; B = temp_v; }

    /* calculate y step per line */
    s.linescale[0] = 1.0f / (C->y - A->y);
    s.linescale[1] = 1.0f / (B->y - A->y);
    s.linescale[2] = 1.0f / (C->y - B->y);

    if( s.linescale[0] <= 0.0f )
        return;

    /* check if the major edge is left or right */
    temp[0] = A->x - C->x;
    temp[1] = A->y - C->y;
    temp[2] = B->x - A->x;
    temp[3] = B->y - A->y;

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
            temp[0] = B->y - A->y;
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
    rs_vertex A, B, C;
    float f;

    /* sanity check */
    if( (ctx->cull_cw && ctx->cull_ccw) || ctx->depth_test==COMPARE_NEVER )
        return;

    if( v0->w<=0.0 || v1->w<=0.0 || v2->w<=0.0 )
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
    if( (int)A.y>ctx->draw_area.maxy && (int)B.y>ctx->draw_area.maxy &&
        (int)C.y>ctx->draw_area.maxy )
    {
        return;
    }
    if( (int)A.x>ctx->draw_area.maxx && (int)B.x>ctx->draw_area.maxx &&
        (int)C.x>ctx->draw_area.maxx )
    {
        return;
    }
    if( (int)A.y<ctx->draw_area.miny && (int)B.y<ctx->draw_area.miny &&
        (int)C.y<ctx->draw_area.miny )
    {
        return;
    }
    if( (int)A.x<ctx->draw_area.minx && (int)B.x<ctx->draw_area.minx &&
        (int)C.x<ctx->draw_area.minx )
    {
        return;
    }

    /* culling */
    f = (C.x - A.x) * (C.y - B.y) - (C.y - A.y) * (C.x - B.x);

    if( (ctx->cull_cw && f>0.0f) || (ctx->cull_ccw && f<0.0f) )
        return;

    draw_triangle( &A, &B, &C, ctx );
}

