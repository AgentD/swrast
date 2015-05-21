#include "inputassembler.h"
#include "rasterizer.h"
#include "context.h"
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

