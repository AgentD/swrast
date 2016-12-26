#include "framebuffer.h"
#include "rasterizer.h"
#include "context.h"
#include "texture.h"
#include "shader.h"
#include <math.h>



typedef struct
{
    vec4 pos;
    vec4 color;
}
rs_vertex_min;

typedef struct
{
    int left;                       /* index of left edge */
    int right;                      /* index of right edge */

    float linescale[3];             /* 1 / dy */

    struct
    {
        rs_vertex_min v;                /* current vertex */
        rs_vertex_min dvdy;             /* difference per line */
    }
    edge[2];
}
edge_data;

typedef struct
{
    float pixelscale;           /* 1.0/dx */

    rs_vertex_min v;                /* current vertex */
    rs_vertex_min dvdx;             /* difference per pixel */
}
scan_line;



static void scaled_vertex_diff( rs_vertex_min* V, const rs_vertex_min* A,
                                const rs_vertex_min* B, float scale )
{
    V->pos = vec4_scale( vec4_sub(A->pos, B->pos), scale);
    V->color = vec4_scale( vec4_sub(A->color, B->color), scale);
}

static void scaled_vertex_add( rs_vertex_min* V, const rs_vertex_min* A,
                               const rs_vertex_min* B, float scale )
{
    V->pos = vec4_add( A->pos, vec4_scale(B->pos, scale) );
    V->color = vec4_add( A->color, vec4_scale(B->color, scale) );
}

/****************************************************************************/

static void draw_scanline( int y, context* ctx, const edge_data* s )
{
    float sub_pixel, z, w, *z_buffer;
    unsigned char *start, *end;
    scan_line l;
    int x0, x1;
    vec4 color;

    /* get line start and end */
    x0 = ceil( s->edge[s->left].v.pos.x );
    x1 = ceil( s->edge[s->right].v.pos.x );

    sub_pixel = (float)x0 - s->edge[s->left].v.pos.x;

    /* compuate start values and slopes */
    l.pixelscale = 1.0f/(s->edge[s->right].v.pos.x -
                         s->edge[s->left].v.pos.x);

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
        z = l.v.pos.z;

        if( depth_test(ctx, z, *z_buffer) )
        {
            w = 1.0f / l.v.pos.w;
            color = vec4_scale(l.v.color, w);

            write_fragment( ctx, color, z, start, z_buffer );
        }

        scaled_vertex_add( &l.v, &l.v, &l.dvdx, 1.0f );
    }
}

static void advance_line( edge_data* s, float scale )
{
    scaled_vertex_add(&s->edge[0].v, &s->edge[0].v, &s->edge[0].dvdy, scale);
    scaled_vertex_add(&s->edge[1].v, &s->edge[1].v, &s->edge[1].dvdy, scale);
}

static void draw_half_triangle( edge_data* s, rs_vertex_min* A,
                                rs_vertex_min* B, context* ctx )
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

void draw_triangle_per_vertex( rs_vertex* v0, rs_vertex* v1,
                               rs_vertex* v2, context* ctx )
{
    rs_vertex_min A, B, C;
    rs_vertex* temp_v;
    float temp[4];
    edge_data s;

    /* sort on Y axis */
    if( v0->attribs[ATTRIB_POS].y > v1->attribs[ATTRIB_POS].y )
    {
        temp_v = v0; v0 = v1; v1 = temp_v;
    }
    if( v1->attribs[ATTRIB_POS].y > v2->attribs[ATTRIB_POS].y )
    {
        temp_v = v1; v1 = v2; v2 = temp_v;
    }
    if( v0->attribs[ATTRIB_POS].y > v1->attribs[ATTRIB_POS].y )
    {
        temp_v = v0; v0 = v1; v1 = temp_v;
    }

    A.pos = v0->attribs[ATTRIB_POS];
    A.color = v0->attribs[ATTRIB_COLOR];
    B.pos = v1->attribs[ATTRIB_POS];
    B.color = v1->attribs[ATTRIB_COLOR];
    C.pos = v2->attribs[ATTRIB_POS];
    C.color = v2->attribs[ATTRIB_COLOR];

    /* calculate y step per line */
    s.linescale[0] = 1.0f/(C.pos.y-A.pos.y);
    s.linescale[1] = 1.0f/(B.pos.y-A.pos.y);
    s.linescale[2] = 1.0f/(C.pos.y-B.pos.y);

    if( s.linescale[0] <= 0.0f )
        return;

    /* check if the major edge is left or right */
    temp[0] = A.pos.x - C.pos.x;
    temp[1] = A.pos.y - C.pos.y;
    temp[2] = B.pos.x - A.pos.x;
    temp[3] = B.pos.y - A.pos.y;

    s.left = (temp[0]*temp[3] - temp[1]*temp[2]) > 0.0f ? 0 : 1;
    s.right = !s.left;

    /* calculate slopes for major edge */
    s.edge[0].v = A;
    scaled_vertex_diff( &s.edge[0].dvdy, &C, &A, s.linescale[0] );

    /* rasterize upper sub-triangle */
    if( s.linescale[1] > 0.0f )
    {
        /* calculate slopes for minor edge */
        s.edge[1].v = A;
        scaled_vertex_diff( &s.edge[1].dvdy, &B, &A, s.linescale[1] );

        /* rasterize the edge scanlines */
        draw_half_triangle( &s, &A, &B, ctx );
    }

    /* rasterize lower sub-triangle */
    if( s.linescale[2] > 0.0f )
    {
        /* advance to center */
        if( s.linescale[1] > 0.0f )
        {
            temp[0] = B.pos.y - A.pos.y;
            scaled_vertex_add( &s.edge[0].v, &A, &s.edge[0].dvdy, temp[0] );
        }

        /* calculate slopes for bottom edge */
        s.edge[1].v = B;
        scaled_vertex_diff( &s.edge[1].dvdy, &C, &B, s.linescale[2] );

        draw_half_triangle( &s, &B, &C, ctx );
    }
}
