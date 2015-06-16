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
    vec4_set( &v->pos, 0.0f, 0.0f, 0.0f, 1.0f );
    vec4_set( &v->color, 1.0f, 1.0f, 1.0f, 1.0f );
    vec4_set( &v->normal, 0.0f, 0.0f, 0.0f, 0.0f );
    vec4_set( &v->texcoord[0], 0.0f, 0.0f, 0.0f, 0.0f );

    /* decode position */
    if( vertex_format & VF_POSITION_F2 )
    {
        vec4_set( &v->pos, ((float*)ptr)[0], ((float*)ptr)[1], 0.0f, 1.0f );
        ptr += 2*sizeof(float);
    }
    else if( vertex_format & VF_POSITION_F3 )
    {
        vec4_set( &v->pos, ((float*)ptr)[0], ((float*)ptr)[1],
                           ((float*)ptr)[2], 1.0f );
        ptr += 3*sizeof(float);
    }
    else if( vertex_format & VF_POSITION_F4 )
    {
        vec4_set( &v->pos, ((float*)ptr)[0], ((float*)ptr)[1],
                           ((float*)ptr)[2], ((float*)ptr)[3] );
        ptr += 4*sizeof(float);
    }

    /* decode surface normal */
    if( vertex_format & VF_NORMAL_F3 )
    {
        vec4_set( &v->normal, ((float*)ptr)[0], ((float*)ptr)[1],
                              ((float*)ptr)[2], 0.0f );
        ptr += 3*sizeof(float);
    }

    /* decode color */
    if( vertex_format & VF_COLOR_F3 )
    {
        vec4_set( &v->color, ((float*)ptr)[0], ((float*)ptr)[1],
                             ((float*)ptr)[2], 1.0f );
        ptr += 3*sizeof(float);
    }
    else if( vertex_format & VF_COLOR_F4 )
    {
        vec4_set( &v->color, ((float*)ptr)[0], ((float*)ptr)[1],
                             ((float*)ptr)[2], ((float*)ptr)[3] );
        ptr += 4*sizeof(float);
    }
    else if( vertex_format & VF_COLOR_UB3 )
    {
        vec4_set( &v->color, ((float)ptr[0])/255.0f, ((float)ptr[1])/255.0f,
                             ((float)ptr[2])/255.0f, 1.0f );
        ptr += 3;
    }
    else if( vertex_format & VF_COLOR_UB4 )
    {
        vec4_set( &v->color, ((float)ptr[0])/255.0f, ((float)ptr[1])/255.0f,
                             ((float)ptr[2])/255.0f, ((float)ptr[3])/255.0f );
        ptr += 4;
    }

    /* decode texture coordinates */
    if( vertex_format & VF_TEX0 )
    {
        vec4_set( &v->texcoord[0], ((float*)ptr)[0], ((float*)ptr)[1],
                                   0.0f, 0.0f );
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

        vec4_set( &ctx->immediate.vertex[i].pos, x, y, z, w );
        ctx->immediate.vertex[i].normal = ctx->immediate.normal;
        ctx->immediate.vertex[i].color = ctx->immediate.color;

        for( j=0; j<MAX_TEXTURES; ++j )
            ctx->immediate.vertex[i].texcoord[j] = ctx->immediate.texcoord[j];

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
        vec4_set( &ctx->immediate.color, r, g, b, a );
        ctx->vertex_format |= VF_COLOR_F4;
    }
}

void ia_normal( context* ctx, float x, float y, float z )
{
    if( ctx->immediate.active )
    {
        vec4_set( &ctx->immediate.normal, x, y, z, 0.0f );
        ctx->vertex_format |= VF_NORMAL_F3;
    }
}

void ia_texcoord( context* ctx, int layer, float s, float t )
{
    if( ctx->immediate.active )
        vec4_set( &ctx->immediate.texcoord[layer], s, t, 0.0f, 0.0f );
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

