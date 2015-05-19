#include "framebuffer.h"
#include "rasterizer.h"
#include "context.h"
#include "texture.h"
#include <math.h>



#define CULL_EPSILON (0.05f)



typedef struct
{
    float x, y, z, w;
    float s[MAX_TEXTURES], t[MAX_TEXTURES];
    float color[4];
}
rs_fragment;

typedef struct
{
    int left;                       /* index of left edge */
    int right;                      /* index of right edge */

    float linescale[3];             /* 1 / dy */

    struct
    {
        float s[MAX_TEXTURES];      /* s texture coordinat / w */
        float dSdY[MAX_TEXTURES];   /* ds/dy */

        float t[MAX_TEXTURES];      /* t texture coordinat / w */
        float dTdY[MAX_TEXTURES];   /* dt/dy */

        float col[4];               /* vertex color / w */
        float dColdY[4];            /* dColor / dy */

        float x;
        float dXdY;                 /* dx/dy */

        float z;                    /* z / w */
        float dZdY;                 /* dz/dy */

        float w;                    /* 1.0 / w */
        float dWdY;                 /* dw/dy */
    }
    edge[2];
}
edge_data;

typedef struct
{
    float pixelscale;           /* 1.0/dx */

    float s[MAX_TEXTURES];      /* s texture coordinate / w */
    float dSdX[MAX_TEXTURES];   /* ds/dx */

    float t[MAX_TEXTURES];      /* t texture coordinate / w */
    float dTdX[MAX_TEXTURES];   /* dt/dx */

    float z;                    /* z / w */
    float dZdX;                 /* dz/dx */

    float w;                    /* 1.0 / w */
    float dWdX;                 /* dw/dx */

    float col[4];               /* vertex color / w */
    float dColdX[4];            /* dColor / dx */
}
scan_line;



static void draw_scanline( int y, context* ctx, const edge_data* s )
{
    int x0, x1, i, j, depthtest[8];
    float sub_pixel, w, *z_buffer;
    unsigned char *start, *end;
    unsigned char tex[4];
    unsigned int c[4];
    scan_line l;

    /* get line start and end */
    x0 = ceil( s->edge[s->left].x );
    x1 = ceil( s->edge[s->right].x );

    if( x1<=x0 )
        return;

    sub_pixel = (float)x0 - s->edge[s->left].x;

    z_buffer = ctx->target->depth + (y*ctx->target->width + x0);
    start = ctx->target->color + (y*ctx->target->width + x0)*4;
    end = ctx->target->color + (y*ctx->target->width + x1)*4;

    /* compuate start values and slopes */
    l.pixelscale = 1.0f / (s->edge[s->right].x - s->edge[s->left].x);

    for( i=0; i<MAX_TEXTURES; ++i )
    {
        l.dSdX[i]=(s->edge[s->right].s[i]-s->edge[s->left].s[i])*l.pixelscale;
        l.dTdX[i]=(s->edge[s->right].t[i]-s->edge[s->left].t[i])*l.pixelscale;

        l.s[i] = s->edge[s->left].s[i] + l.dSdX[i] * sub_pixel;
        l.t[i] = s->edge[s->left].t[i] + l.dTdX[i] * sub_pixel;
    }

    l.dWdX = (s->edge[s->right].w - s->edge[s->left].w) * l.pixelscale;
    l.dZdX = (s->edge[s->right].z - s->edge[s->left].z) * l.pixelscale;

    l.z = s->edge[s->left].z + l.dZdX * sub_pixel;
    l.w = s->edge[s->left].w + l.dWdX * sub_pixel;

    for( i=0; i<4; ++i )
    {
        l.dColdX[i] = (s->edge[s->right].col[i] - s->edge[s->left].col[i]) *
                      l.pixelscale;
        l.col[i] = s->edge[s->left].col[i] + l.dColdX[i] * sub_pixel;
    }

    /* for each fragment */
    for( ; start!=end && x0<=ctx->draw_area.maxx; start+=4, ++z_buffer, ++x0 )
    {
        if( x0<=ctx->draw_area.minx )
            goto skip_fragment;

        /* depth test */
        if( ctx->depth_test!=COMPARE_ALWAYS )
        {
            depthtest[COMPARE_LESS         ] = (l.z < *z_buffer);
            depthtest[COMPARE_GREATER      ] = (l.z > *z_buffer);
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

        w = 1.0f / l.w;

        /* interpolate colors */
        for( i=0; i<4; ++i )
            c[i] = (unsigned char)(l.col[i] * w);

        /* interpolate texture coordinates, fetch texture colors */
        for( i=0; i<MAX_TEXTURES; ++i )
        {
            if( ctx->texture_enable[ i ] )
            {
                texture_sample( ctx->textures[i], l.s[i]*w, l.t[i]*w, tex );

                for( j=0; j<4; ++j )
                    c[j] = (c[j]*tex[j]) >> 8;
            }
        }

        /* blend onto framebuffer color */
        if( ctx->alpha_blend )
        {
            unsigned int a = c[ALPHA], ia = 0xFF - a;

            c[RED  ] = (start[RED  ]*ia + c[RED  ]*a) >> 8;
            c[GREEN] = (start[GREEN]*ia + c[GREEN]*a) >> 8;
            c[BLUE ] = (start[BLUE ]*ia + c[BLUE ]*a) >> 8;
            c[ALPHA] = (start[ALPHA]*ia + (a<<8)    ) >> 8;
        }

        for( i=0; i<4; ++i )
            start[i] = c[i];

        *z_buffer = l.z;

    skip_fragment:
        for( i=0; i<4; ++i )
            l.col[i] += l.dColdX[i];

        for( i=0; i<MAX_TEXTURES; ++i )
        {
            l.s[i] += l.dSdX[i];
            l.t[i] += l.dTdX[i];
        }

        l.w += l.dWdX;
        l.z += l.dZdX;
    }
}

static void advance_line( edge_data* s, float scale )
{
    int i, j;

    for( i=0; i<2; ++i)
    {
        s->edge[i].x += s->edge[i].dXdY * scale;
        s->edge[i].z += s->edge[i].dZdY * scale;
        s->edge[i].w += s->edge[i].dWdY * scale;

        for( j=0; j<4; ++j )
            s->edge[i].col[j] += s->edge[i].dColdY[j] * scale;

        for( j=0; j<MAX_TEXTURES; ++j )
        {
            s->edge[i].s[j] += s->edge[i].dSdY[j] * scale;
            s->edge[i].t[j] += s->edge[i].dTdY[j] * scale;
        }
    }
}

static void draw_triangle( rs_fragment* A, rs_fragment* B,
                           rs_fragment* C, context* ctx )
{
    rs_fragment* temp_v;
    int y, y0, y1, i;
    float temp[4];
    edge_data s;

    /* sort on Y axis */
    if( A->y > B->y ) { temp_v = A; A = B; B = temp_v; }
    if( B->y > C->y ) { temp_v = B; B = C; C = temp_v; }
    if( A->y > B->y ) { temp_v = A; A = B; B = temp_v; }

    /* calculate delta y of the edges */
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
    s.edge[0].dXdY = (C->x - A->x) * s.linescale[0];
    s.edge[0].dWdY = (C->w - A->w) * s.linescale[0];
    s.edge[0].dZdY = (C->z - A->z) * s.linescale[0];

    for( i=0; i<MAX_TEXTURES; ++i )
    {
        s.edge[0].dSdY[i] = (C->s[i] - A->s[i]) * s.linescale[0];
        s.edge[0].dTdY[i] = (C->t[i] - A->t[i]) * s.linescale[0];

        s.edge[0].s[i] = A->s[i];
        s.edge[0].t[i] = A->t[i];
    }

    s.edge[0].x = A->x;
    s.edge[0].w = A->w;
    s.edge[0].z = A->z;

    for( i=0; i<4; ++i )
    {
        s.edge[0].col[i] = A->color[i];
        s.edge[0].dColdY[i] = (C->color[i] - A->color[i]) * s.linescale[0];
    }

    /* rasterize upper sub-triangle */
    if( s.linescale[1] > 0.0f )
    {
        /* calculate slopes for minor edge */
        s.edge[1].dXdY = (B->x - A->x) * s.linescale[1];
        s.edge[1].dWdY = (B->w - A->w) * s.linescale[1];
        s.edge[1].dZdY = (B->z - A->z) * s.linescale[1];

        s.edge[1].x = A->x;
        s.edge[1].w = A->w;
        s.edge[1].z = A->z;

        for( i=0; i<4; ++i )
        {
            s.edge[1].col[i] = A->color[i];
            s.edge[1].dColdY[i] = (B->color[i] - A->color[i]) * s.linescale[1];
        }

        for( i=0; i<MAX_TEXTURES; ++i )
        {
            s.edge[1].dSdY[i] = (B->s[i] - A->s[i]) * s.linescale[1];
            s.edge[1].dTdY[i] = (B->t[i] - A->t[i]) * s.linescale[1];

            s.edge[1].s[i] = A->s[i];
            s.edge[1].t[i] = A->t[i];
        }

        /* apply top-left fill convention */
        y0 = ceil( A->y );
        y1 = ceil( B->y ) - 1;

        advance_line( &s, (float)y0 - A->y );

        /* rasterize the edge scanlines */
        for( y=y0; y<=y1 && y<=ctx->draw_area.maxy; ++y )
        {
            if( y>=ctx->draw_area.miny )
                draw_scanline( y, ctx, &s );
            advance_line( &s, 1.0f );
        }
    }

    /* rasterize lower sub-triangle */
    if( s.linescale[2] > 0.0f )
    {
        /* advance to center */
        if( s.linescale[1] > 0.0f )
        {
            temp[0] = B->y - A->y;

            s.edge[0].x = A->x + s.edge[0].dXdY * temp[0];
            s.edge[0].z = A->z + s.edge[0].dZdY * temp[0];
            s.edge[0].w = A->w + s.edge[0].dWdY * temp[0];

            for( i=0; i<4; ++i)
                s.edge[0].col[i] = A->color[i] + s.edge[0].dColdY[i]*temp[0];

            for( i=0; i<MAX_TEXTURES; ++i )
            {
                s.edge[0].s[i] = A->s[i] + s.edge[0].dSdY[i] * temp[0];
                s.edge[0].t[i] = A->t[i] + s.edge[0].dTdY[i] * temp[0];
            }
        }

        /* calculate slopes for bottom edge */
        s.edge[1].dXdY = (C->x - B->x) * s.linescale[2];
        s.edge[1].dWdY = (C->w - B->w) * s.linescale[2];
        s.edge[1].dZdY = (C->z - B->z) * s.linescale[2];

        s.edge[1].x = B->x;
        s.edge[1].w = B->w;
        s.edge[1].z = B->z;

        for( i=0; i<MAX_TEXTURES; ++i )
        {
            s.edge[1].dSdY[i] = (C->s[i] - B->s[i]) * s.linescale[2];
            s.edge[1].dTdY[i] = (C->t[i] - B->t[i]) * s.linescale[2];
            s.edge[1].s[i] = B->s[i];
            s.edge[1].t[i] = B->t[i];
        }

        for( i=0; i<4; ++i )
        {
            s.edge[1].col[i] = B->color[i];
            s.edge[1].dColdY[i] = (C->color[i] - B->color[i])*s.linescale[2];
        }

        /* apply top-left fill convention */
        y0 = ceil( B->y );
        y1 = ceil( C->y ) - 1;

        advance_line( &s, (float)y0 - B->y );

        /* draw scanlines */
        for( y=y0; y<=y1 && y<=ctx->draw_area.maxy; ++y )
        {
            if( y>=ctx->draw_area.miny )
                draw_scanline( y, ctx, &s );
            advance_line( &s, 1.0f );
        }
    }
}

void rasterizer_process_triangle( context* ctx, const rs_vertex* v0,
                                  const rs_vertex* v1, const rs_vertex* v2 )
{
    rs_fragment A, B, C;
    float f;
    int i;

    /* sanity check */
    if( ctx->cull_cw && ctx->cull_ccw )
        return;

    if( v0->w<=0.0 || v1->w<=0.0 || v2->w<=0.0 )
        return;

    if( ctx->depth_test==COMPARE_NEVER )
        return;

    if( ctx->draw_area.minx>=ctx->draw_area.maxx ||
        ctx->draw_area.miny>=ctx->draw_area.maxy )
    {
        return;
    }

    /* prepare vertex positions */
    A.w = 1.0/v0->w;
    A.x = (1.0f + v0->x*A.w) * 0.5f * (float)ctx->viewport.width;
    A.y = (1.0f - v0->y*A.w) * 0.5f * (float)ctx->viewport.height;
    A.z = (1.0f - v0->z*A.w) * 0.5f;

    B.w = 1.0/v1->w;
    B.x = (1.0f + v1->x*B.w) * 0.5f * (float)ctx->viewport.width;
    B.y = (1.0f - v1->y*B.w) * 0.5f * (float)ctx->viewport.height;
    B.z = (1.0f - v1->z*B.w) * 0.5f;

    C.w = 1.0/v2->w;
    C.x = (1.0f + v2->x*C.w) * 0.5f * (float)ctx->viewport.width;
    C.y = (1.0f - v2->y*C.w) * 0.5f * (float)ctx->viewport.height;
    C.z = (1.0f - v2->z*C.w) * 0.5f;

    A.x += ctx->viewport.x;
    B.x += ctx->viewport.x;
    C.x += ctx->viewport.x;

    A.y += ctx->viewport.y;
    B.y += ctx->viewport.y;
    C.y += ctx->viewport.y;

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
    f = (v2->x - v0->x) * (v2->y - v1->y) - (v2->y - v0->y) * (v2->x - v1->x);

    if( ctx->cull_cw && f<-CULL_EPSILON )
        return;
    if( ctx->cull_ccw && f>CULL_EPSILON )
        return;

    /* prepare vertex colors */
    A.color[RED] = v0->r * A.w;
    A.color[GREEN] = v0->g * A.w;
    A.color[BLUE] = v0->b * A.w;
    A.color[ALPHA] = v0->a * A.w;

    B.color[RED] = v1->r * B.w;
    B.color[GREEN] = v1->g * B.w;
    B.color[BLUE] = v1->b * B.w;
    B.color[ALPHA] = v1->a * B.w;

    C.color[RED] = v2->r * C.w;
    C.color[GREEN] = v2->g * C.w;
    C.color[BLUE] = v2->b * C.w;
    C.color[ALPHA] = v2->a * C.w;

    /* prepare texture coordinates */
    for( i=0; i<MAX_TEXTURES; ++i )
    {
        A.s[i] = v0->s[i] * A.w;
        A.t[i] = v0->t[i] * A.w;

        B.s[i] = v1->s[i] * B.w;
        B.t[i] = v1->t[i] * B.w;

        C.s[i] = v2->s[i] * C.w;
        C.t[i] = v2->t[i] * C.w;
    }

    draw_triangle( &A, &B, &C, ctx );
}

