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
    v->r = 0xFF;
    v->g = 0xFF;
    v->b = 0xFF;
    v->a = 0xFF;
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
        v->r = ((float*)ptr)[0] * 255.0f;
        v->g = ((float*)ptr)[1] * 255.0f;
        v->b = ((float*)ptr)[2] * 255.0f;
        ptr += 3*sizeof(float);
    }
    else if( vertex_format & VF_COLOR_F4 )
    {
        v->r = ((float*)ptr)[0] * 255.0f;
        v->g = ((float*)ptr)[1] * 255.0f;
        v->b = ((float*)ptr)[2] * 255.0f;
        v->a = ((float*)ptr)[3] * 255.0f;
        ptr += 4*sizeof(float);
    }
    else if( vertex_format & VF_COLOR_UB3 )
    {
        v->r = ptr[0];
        v->g = ptr[1];
        v->b = ptr[2];
        ptr += 3;
    }
    else if( vertex_format & VF_COLOR_UB4 )
    {
        v->r = ptr[0];
        v->g = ptr[1];
        v->b = ptr[2];
        v->a = ptr[3];
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

static void draw_triangle( context* ctx, rs_vertex* v0,
                           rs_vertex* v1, rs_vertex* v2 )
{
    rs_vertex provoking;
    float u[4], v[4], s;

    if( ctx->shade_model==SHADE_GOURAUD )
    {
        tl_transform_and_light_vertex( ctx, v0, 0 );
        tl_transform_and_light_vertex( ctx, v1, 0 );
        tl_transform_and_light_vertex( ctx, v2, 0 );
    }
    else if( ctx->shade_model==SHADE_FLAT )
    {
        /* get position of provoking vertex */
        switch( ctx->provoking_vertex )
        {
        case 0: provoking.x=v0->x; provoking.y=v0->y;
                provoking.z=v0->z; provoking.w=v0->w;
                break;
        case 1: provoking.x=v1->x; provoking.y=v1->y;
                provoking.z=v1->z; provoking.w=v1->w;
                break;
        case 2: provoking.x=v2->x; provoking.y=v2->y;
                provoking.z=v2->z; provoking.w=v2->w;
                break;
        }

        /* compute normal for triangle */
        u[0] = v1->x - v0->x; u[1] = v1->y - v0->y; u[2] = v1->z - v0->z;
        v[0] = v2->x - v0->x; v[1] = v2->y - v0->y; v[2] = v2->z - v0->z;

        provoking.nx = u[1]*v[2] - u[2]*v[1];
        provoking.ny = u[2]*v[0] - u[0]*v[2];
        provoking.nz = u[0]*v[1] - u[1]*v[0];
        s = provoking.nx*provoking.nx + provoking.ny*provoking.ny +
            provoking.nz*provoking.nz;

        if( !ctx->front_is_ccw )
            s *= -1.0f;

        s = 1.0f / sqrt( s );
        provoking.nx *= s;
        provoking.ny *= s;
        provoking.nz *= s;

        /* compute lighting for provoking vertex */
        provoking.r=0xFF;provoking.g=0xFF;provoking.b=0xFF;provoking.a=0xFF;
        tl_transform_and_light_vertex( ctx, &provoking, 0 );

        /* transform vertices */
        tl_transform_and_light_vertex( ctx, v0, 1 );
        tl_transform_and_light_vertex( ctx, v1, 1 );
        tl_transform_and_light_vertex( ctx, v2, 1 );

        /* apply lighting */
        v0->r = (v0->r*provoking.r)>>8;
        v0->g = (v0->g*provoking.g)>>8;
        v0->b = (v0->b*provoking.b)>>8;
        v0->a = (v0->a*provoking.a)>>8;

        v1->r = (v1->r*provoking.r)>>8;
        v1->g = (v1->g*provoking.g)>>8;
        v1->b = (v1->b*provoking.b)>>8;
        v1->a = (v1->a*provoking.a)>>8;

        v2->r = (v2->r*provoking.r)>>8;
        v2->g = (v2->g*provoking.g)>>8;
        v2->b = (v2->b*provoking.b)>>8;
        v2->a = (v2->a*provoking.a)>>8;
    }

    rasterizer_process_triangle( ctx, v0, v1, v2 );
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
        draw_triangle( ctx, &v0, &v1, &v2 );
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
        draw_triangle( ctx, &v0, &v1, &v2 );
    }
}

