#include "inputassembler.h"
#include "rasterizer.h"
#include "tl.h"


static int vertex_format;



/****************************************************************************
 *                         state handling functions                         *
 ****************************************************************************/

void ia_set_vertex_format( int format )
{
    vertex_format = format;
}

/****************************************************************************
 *                      triangle processing functions                       *
 ****************************************************************************/

static unsigned char* read_vertex( rs_vertex* v, unsigned char* ptr )
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

void ia_draw_triangles( framebuffer* fb, void* ptr, unsigned int vertexcount )
{
    unsigned int i;
    rs_triangle t;

    vertexcount -= vertexcount % 3;

    for( i=0; i<vertexcount; i+=3 )
    {
        ptr = read_vertex( &(t.v0), ptr );
        ptr = read_vertex( &(t.v1), ptr );
        ptr = read_vertex( &(t.v2), ptr );
        tl_transform_and_light_triangle( &t );
        rasterizer_process_triangle( &t, fb );
    }
}

void ia_draw_triangles_indexed( framebuffer* fb, void* ptr,
                                unsigned int vertexcount,
                                unsigned short* indices,
                                unsigned int indexcount )
{
    unsigned int vsize = 0;
    unsigned short index;
    unsigned int i;
    rs_triangle t;

    /* determine vertex size in bytes */
         if( vertex_format & VF_POSITION_F2 ) { vsize += 2*sizeof(float); }
    else if( vertex_format & VF_POSITION_F3 ) { vsize += 3*sizeof(float); }
    else if( vertex_format & VF_POSITION_F4 ) { vsize += 4*sizeof(float); }

         if( vertex_format & VF_NORMAL_F3 ) { vsize += 3*sizeof(float); }

         if( vertex_format & VF_COLOR_F3  ) { vsize += 3*sizeof(float); }
    else if( vertex_format & VF_COLOR_F4  ) { vsize += 4*sizeof(float); }
    else if( vertex_format & VF_COLOR_UB3 ) { vsize += 3;               }
    else if( vertex_format & VF_COLOR_UB4 ) { vsize += 4;               }

    if( vertex_format & VF_TEX0 )
        vsize += 2*sizeof(float);

    /* for each triangle */
    indexcount -= indexcount % 3;

    for( i=0; i<indexcount; i+=3 )
    {
        /* read first vertex */
        index = indices[ i ];

        if( index>=vertexcount )
            continue;
        
        read_vertex( &(t.v0), ((unsigned char*)ptr) + vsize*index );

        /* read second vertex */
        index = indices[ i+1 ];

        if( index>=vertexcount )
            continue;

        read_vertex( &(t.v1), ((unsigned char*)ptr) + vsize*index );

        /* read third vertex */
        index = indices[ i+2 ];

        if( index>=vertexcount )
            continue;

        read_vertex( &(t.v2), ((unsigned char*)ptr) + vsize*index );

        /* rasterize */
        tl_transform_and_light_triangle( &t );
        rasterizer_process_triangle( &t, fb );
    }
}

