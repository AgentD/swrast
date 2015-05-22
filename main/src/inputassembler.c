#include "inputassembler.h"
#include "rasterizer.h"
#include "context.h"
#include "config.h"
#include "tl.h"
#include <math.h>


static unsigned char* read_vertex( rs_vertex* v, unsigned char* ptr,
                                   int vertex_format )
{
    /* initialize vertex structure */
    v->x = 0.0;
    v->y = 0.0;
    v->z = 0.0;
    v->w = 1.0;
    v->r = 1.0f;
    v->g = 1.0f;
    v->b = 1.0f;
    v->a = 1.0f;
    v->nx = 0.0;
    v->ny = 0.0;
    v->nz = 0.0;
    v->s[0] = 0.0;
    v->t[0] = 0.0;

    /* decode position */
    if( vertex_format & VF_POSITION_F2 )
    {
        v->x = ((float*)ptr)[0];
        v->y = ((float*)ptr)[1];
        ptr += 2*sizeof(float);
    }
    else if( vertex_format & VF_POSITION_F3 )
    {
        v->x = ((float*)ptr)[0];
        v->y = ((float*)ptr)[1];
        v->z = ((float*)ptr)[2];
        ptr += 3*sizeof(float);
    }
    else if( vertex_format & VF_POSITION_F4 )
    {
        v->x = ((float*)ptr)[0];
        v->y = ((float*)ptr)[1];
        v->z = ((float*)ptr)[2];
        v->w = ((float*)ptr)[3];
        ptr += 4*sizeof(float);
    }

    /* decode surface normal */
    if( vertex_format & VF_NORMAL_F3 )
    {
        v->nx = ((float*)ptr)[0];
        v->ny = ((float*)ptr)[1];
        v->nz = ((float*)ptr)[2];
        ptr += 3*sizeof(float);
    }

    /* decode color */
    if( vertex_format & VF_COLOR_F3 )
    {
        v->r = ((float*)ptr)[0];
        v->g = ((float*)ptr)[1];
        v->b = ((float*)ptr)[2];
        ptr += 3*sizeof(float);
    }
    else if( vertex_format & VF_COLOR_F4 )
    {
        v->r = ((float*)ptr)[0];
        v->g = ((float*)ptr)[1];
        v->b = ((float*)ptr)[2];
        v->a = ((float*)ptr)[3];
        ptr += 4*sizeof(float);
    }
    else if( vertex_format & VF_COLOR_UB3 )
    {
        v->r = ((float)ptr[0]) / 255.0f;
        v->g = ((float)ptr[1]) / 255.0f;
        v->b = ((float)ptr[2]) / 255.0f;
        ptr += 3;
    }
    else if( vertex_format & VF_COLOR_UB4 )
    {
        v->r = ((float)ptr[0]) / 255.0f;
        v->g = ((float)ptr[1]) / 255.0f;
        v->b = ((float)ptr[2]) / 255.0f;
        v->a = ((float)ptr[3]) / 255.0f;
        ptr += 4;
    }

    /* decode texture coordinates */
    if( vertex_format & VF_TEX0 )
    {
        v->s[0] = ((float*)ptr)[0];
        v->t[0] = ((float*)ptr)[1];
        ptr += 2*sizeof(float);
    }

    return ptr;
}

void ia_draw_triangles( context* ctx, unsigned int vertexcount )
{
    void* ptr = ctx->vertexbuffer;
    rs_vertex v0, v1, v2;
    unsigned int i;

    if( ctx->immediate.active )
        return;

    for( i=0; i<vertexcount; i+=3 )
    {
        ptr = read_vertex( &v0, ptr, ctx->vertex_format );
        ptr = read_vertex( &v1, ptr, ctx->vertex_format );
        ptr = read_vertex( &v2, ptr, ctx->vertex_format );
        tl_transform_and_light( ctx, &v0, &v1, &v2 );
        rasterizer_process_triangle( ctx, &v0, &v1, &v2 );
    }
}

void ia_draw_triangles_indexed( context* ctx, unsigned int vertexcount,
                                unsigned int indexcount )
{
    unsigned int vsize = 0;
    rs_vertex v0, v1, v2;
    unsigned short index;
    unsigned int i;

    if( ctx->immediate.active )
        return;

    /* determine vertex size in bytes */
         if(ctx->vertex_format & VF_POSITION_F2) { vsize += 2*sizeof(float); }
    else if(ctx->vertex_format & VF_POSITION_F3) { vsize += 3*sizeof(float); }
    else if(ctx->vertex_format & VF_POSITION_F4) { vsize += 4*sizeof(float); }

         if( ctx->vertex_format & VF_NORMAL_F3 ) { vsize += 3*sizeof(float); }

         if( ctx->vertex_format & VF_COLOR_F3  ) { vsize += 3*sizeof(float); }
    else if( ctx->vertex_format & VF_COLOR_F4  ) { vsize += 4*sizeof(float); }
    else if( ctx->vertex_format & VF_COLOR_UB3 ) { vsize += 3;               }
    else if( ctx->vertex_format & VF_COLOR_UB4 ) { vsize += 4;               }

         if( ctx->vertex_format & VF_TEX0      ) { vsize += 2*sizeof(float); }

    /* for each triangle */
    indexcount -= indexcount % 3;

    for( i=0; i<indexcount; i+=3 )
    {
        /* read first vertex */
        index = ctx->indexbuffer[ i ];

        if( index>=vertexcount )
            continue;
        
        read_vertex( &(v0), ((unsigned char*)ctx->vertexbuffer)+vsize*index,
                     ctx->vertex_format );

        /* read second vertex */
        index = ctx->indexbuffer[ i+1 ];

        if( index>=vertexcount )
            continue;

        read_vertex( &(v1), ((unsigned char*)ctx->vertexbuffer)+vsize*index,
                     ctx->vertex_format );

        /* read third vertex */
        index = ctx->indexbuffer[ i+2 ];

        if( index>=vertexcount )
            continue;

        read_vertex( &(v2), ((unsigned char*)ctx->vertexbuffer)+vsize*index,
                     ctx->vertex_format );

        /* rasterize */
        tl_transform_and_light( ctx, &v0, &v1, &v2 );
        rasterizer_process_triangle( ctx, &v0, &v1, &v2 );
    }
}

void ia_begin( context* ctx )
{
    if( !ctx->immediate.active )
    {
        ctx->vertex_format = 0;
        ctx->immediate.current = 0;
        ctx->immediate.active = 1;
    }
}

void ia_vertex( context* ctx, float x, float y, float z, float w )
{
    int i, j;

    if( ctx->immediate.active )
    {
        i = ctx->immediate.current++;

        ctx->immediate.vertex[ i ].x = x;
        ctx->immediate.vertex[ i ].y = y;
        ctx->immediate.vertex[ i ].z = z;
        ctx->immediate.vertex[ i ].w = w;
        ctx->immediate.vertex[ i ].nx = ctx->immediate.normal[0];
        ctx->immediate.vertex[ i ].ny = ctx->immediate.normal[1];
        ctx->immediate.vertex[ i ].nz = ctx->immediate.normal[2];
        ctx->immediate.vertex[ i ].r = ctx->immediate.color[0];
        ctx->immediate.vertex[ i ].g = ctx->immediate.color[1];
        ctx->immediate.vertex[ i ].b = ctx->immediate.color[2];
        ctx->immediate.vertex[ i ].a = ctx->immediate.color[3];

        for( j=0; j<MAX_TEXTURES; ++j )
        {
            ctx->immediate.vertex[ i ].s[ j ] = ctx->immediate.s[ j ];
            ctx->immediate.vertex[ i ].t[ j ] = ctx->immediate.t[ j ];
        }

        ctx->vertex_format |= VF_POSITION_F4;

        if( ctx->immediate.current == 3 )
        {
            tl_transform_and_light( ctx, ctx->immediate.vertex,
                                         ctx->immediate.vertex+1,
                                         ctx->immediate.vertex+2 );
            rasterizer_process_triangle( ctx, ctx->immediate.vertex,
                                              ctx->immediate.vertex+1,
                                              ctx->immediate.vertex+2 );
            ctx->immediate.current = 0;
        }
    }
}

void ia_color( context* ctx, float r, float g, float b, float a )
{
    if( ctx->immediate.active )
    {
        ctx->immediate.color[0] = r;
        ctx->immediate.color[1] = g;
        ctx->immediate.color[2] = b;
        ctx->immediate.color[3] = a;
        ctx->vertex_format |= VF_COLOR_F4;
    }
}

void ia_normal( context* ctx, float x, float y, float z )
{
    if( ctx->immediate.active )
    {
        ctx->immediate.normal[0] = x;
        ctx->immediate.normal[1] = y;
        ctx->immediate.normal[2] = z;
        ctx->vertex_format |= VF_NORMAL_F3;
    }
}

void ia_texcoord( context* ctx, int layer, float s, float t )
{
    if( ctx->immediate.active )
    {
        ctx->immediate.s[layer] = s;
        ctx->immediate.t[layer] = t;
    }
}

void ia_end( context* ctx )
{
    if( ctx->immediate.active )
    {
        ctx->vertex_format = 0;
        ctx->immediate.current = 0;
        ctx->immediate.active = 0;
    }
}

