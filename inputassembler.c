#include "inputassembler.h"
#include "rasterizer.h"

#include <float.h>
#include <math.h>


static float mv[ 16 ];      /* model view matrix */
static float normal[ 16 ];  /* normal matrix */
static float proj[ 16 ];    /* projection matrix */
static int vertex_format;



static void recompute_normal_matrix( void )
{
    float det, m[16];

    /* m = inverse( mv ) */
    m[0] = mv[5]*mv[10]*mv[15]-mv[ 5]*mv[11]*mv[14]-mv[ 9]*mv[6]*mv[15]+
           mv[9]*mv[ 7]*mv[14]+mv[13]*mv[ 6]*mv[11]-mv[13]*mv[7]*mv[10];

    m[1] = -mv[1]*mv[10]*mv[15]+mv[ 1]*mv[11]*mv[14]+mv[ 9]*mv[2]*mv[15]-
            mv[9]*mv[ 3]*mv[14]-mv[13]*mv[ 2]*mv[11]+mv[13]*mv[3]*mv[10];

    m[2] = mv[1]*mv[6]*mv[15]-mv[ 1]*mv[7]*mv[14]-mv[ 5]*mv[2]*mv[15]+
           mv[5]*mv[3]*mv[14]+mv[13]*mv[2]*mv[ 7]-mv[13]*mv[3]*mv[ 6];

    m[3] = -mv[1]*mv[6]*mv[11]+mv[1]*mv[7]*mv[10]+mv[5]*mv[2]*mv[11]-
            mv[5]*mv[3]*mv[10]-mv[9]*mv[2]*mv[ 7]+mv[9]*mv[3]*mv[ 6];

    m[4] = -mv[4]*mv[10]*mv[15]+mv[ 4]*mv[11]*mv[14]+mv[ 8]*mv[6]*mv[15]-
            mv[8]*mv[ 7]*mv[14]-mv[12]*mv[ 6]*mv[11]+mv[12]*mv[7]*mv[10];

    m[5] = mv[0]*mv[10]*mv[15]-mv[ 0]*mv[11]*mv[14]-mv[ 8]*mv[2]*mv[15]+
           mv[8]*mv[ 3]*mv[14]+mv[12]*mv[ 2]*mv[11]-mv[12]*mv[3]*mv[10];

    m[6] = -mv[0]*mv[6]*mv[15]+mv[ 0]*mv[7]*mv[14]+mv[ 4]*mv[2]*mv[15]-
            mv[4]*mv[3]*mv[14]-mv[12]*mv[2]*mv[ 7]+mv[12]*mv[3]*mv[ 6];

    m[7] = mv[0]*mv[6]*mv[11]-mv[0]*mv[7]*mv[10]-mv[4]*mv[2]*mv[11]+
           mv[4]*mv[3]*mv[10]+mv[8]*mv[2]*mv[ 7]-mv[8]*mv[3]*mv[ 6];

    m[8] = mv[4]*mv[9]*mv[15]-mv[ 4]*mv[11]*mv[13]-mv[ 8]*mv[5]*mv[15]+
           mv[8]*mv[7]*mv[13]+mv[12]*mv[ 5]*mv[11]-mv[12]*mv[7]*mv[ 9];

    m[9] = -mv[0]*mv[9]*mv[15]+mv[ 0]*mv[11]*mv[13]+mv[ 8]*mv[1]*mv[15]-
            mv[8]*mv[3]*mv[13]-mv[12]*mv[ 1]*mv[11]+mv[12]*mv[3]*mv[ 9];

    m[10] = mv[0]*mv[5]*mv[15]-mv[ 0]*mv[7]*mv[13]-mv[ 4]*mv[1]*mv[15] +
            mv[4]*mv[3]*mv[13]+mv[12]*mv[1]*mv[ 7]-mv[12]*mv[3]*mv[ 5];

    m[11] = -mv[0]*mv[5]*mv[11]+mv[0]*mv[7]*mv[9]+mv[4]*mv[1]*mv[11]-
             mv[4]*mv[3]*mv[ 9]-mv[8]*mv[1]*mv[7]+mv[8]*mv[3]*mv[ 5];

    m[12] = -mv[4]*mv[9]*mv[14]+mv[ 4]*mv[10]*mv[13]+mv[ 8]*mv[5]*mv[14]-
             mv[8]*mv[6]*mv[13]-mv[12]*mv[ 5]*mv[10]+mv[12]*mv[6]*mv[ 9];

    m[13] = mv[0]*mv[9]*mv[14]-mv[ 0]*mv[10]*mv[13]-mv[ 8]*mv[1]*mv[14]+
            mv[8]*mv[2]*mv[13]+mv[12]*mv[ 1]*mv[10]-mv[12]*mv[2]*mv[ 9];

    m[14] = -mv[0]*mv[5]*mv[14]+mv[ 0]*mv[6]*mv[13]+mv[ 4]*mv[1]*mv[14]-
             mv[4]*mv[2]*mv[13]-mv[12]*mv[1]*mv[ 6]+mv[12]*mv[2]*mv[ 5];

    m[15] = mv[0]*mv[5]*mv[10]-mv[0]*mv[6]*mv[9]-mv[4]*mv[1]*mv[10]+
            mv[4]*mv[2]*mv[ 9]+mv[8]*mv[1]*mv[6]-mv[8]*mv[2]*mv[ 5];

    det = mv[0]*m[0] + mv[1]*m[ 4] + mv[2]*m[8] + mv[3]*m[12];

    if( (det < -FLT_MIN) || (det > FLT_MIN) )
    {
        det = 1.0f / det;
        m[0] *= det; m[4] *= det; m[ 8] *= det; m[12] *= det;
        m[1] *= det; m[5] *= det; m[ 9] *= det; m[13] *= det;
        m[2] *= det; m[6] *= det; m[10] *= det; m[14] *= det;
        m[3] *= det; m[7] *= det; m[11] *= det; m[15] *= det;
    }
    else
    {
        m[0]=1.0f; m[4]=0.0f; m[ 8]=0.0f; m[12]=0.0f;
        m[1]=0.0f; m[5]=1.0f; m[ 9]=0.0f; m[13]=0.0f;
        m[2]=0.0f; m[6]=0.0f; m[10]=1.0f; m[14]=0.0f;
        m[3]=0.0f; m[7]=0.0f; m[11]=0.0f; m[15]=1.0f;
    }

    /* normal = transpose(m) = transpose(inverse(mv)) */
    normal[0]=m[ 0]; normal[4]=m[ 1]; normal[ 8]=m[ 2]; normal[12]=m[ 3];
    normal[1]=m[ 4]; normal[5]=m[ 5]; normal[ 9]=m[ 6]; normal[13]=m[ 7];
    normal[2]=m[ 8]; normal[6]=m[ 9]; normal[10]=m[10]; normal[14]=m[11];
    normal[3]=m[12]; normal[7]=m[13]; normal[11]=m[14]; normal[15]=m[15];
}



/****************************************************************************
 *                         state handling functions                         *
 ****************************************************************************/

void ia_set_modelview_matrix( float* f )
{
    int i;

    for( i=0; i<16; ++i )
        mv[ i ] = f[ i ];

    recompute_normal_matrix( );
}

void ia_set_projection_matrix( float* f )
{
    int i;

    for( i=0; i<16; ++i )
        proj[ i ] = f[ i ];
}

void ia_set_vertex_format( int format )
{
    vertex_format = format;
}

/****************************************************************************
 *                      triangle processing functions                       *
 ****************************************************************************/

static unsigned char* read_vertex( vertex* v, unsigned char* ptr )
{
    /* initialize vertex structure */
    v->x = 0.0;
    v->y = 0.0;
    v->z = 0.0;
    v->w = 1.0;
    v->r = 1.0;
    v->g = 1.0;
    v->b = 1.0;
    v->a = 1.0;
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
        v->r = (float)(ptr[0]) / 255.0;
        v->g = (float)(ptr[1]) / 255.0;
        v->b = (float)(ptr[2]) / 255.0;
        ptr += 3;
    }
    else if( vertex_format & VF_COLOR_UB4 )
    {
        v->r = (float)(ptr[0]) / 255.0;
        v->g = (float)(ptr[1]) / 255.0;
        v->b = (float)(ptr[2]) / 255.0;
        v->a = (float)(ptr[3]) / 255.0;
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

static void transform_vertex( vertex* v )
{
    float x, y, z, w;

    /* position' = projection * modelview * position */
    x = mv[0]*v->x + mv[4]*v->y + mv[ 8]*v->z + mv[12]*v->w;
    y = mv[1]*v->x + mv[5]*v->y + mv[ 9]*v->z + mv[13]*v->w;
    z = mv[2]*v->x + mv[6]*v->y + mv[10]*v->z + mv[14]*v->w;
    w = mv[3]*v->x + mv[7]*v->y + mv[11]*v->z + mv[15]*v->w;

    v->x = proj[0]*x + proj[4]*y + proj[ 8]*z + proj[12]*w;
    v->y = proj[1]*x + proj[5]*y + proj[ 9]*z + proj[13]*w;
    v->z = proj[2]*x + proj[6]*y + proj[10]*z + proj[14]*w;
    v->w = proj[3]*x + proj[7]*y + proj[11]*z + proj[15]*w;

    /* normal' = normalize( normalmatrix * normal ) */
    x = normal[0]*v->nx + normal[4]*v->ny + normal[ 8]*v->nz;
    y = normal[1]*v->nx + normal[5]*v->ny + normal[ 9]*v->nz;
    z = normal[2]*v->nx + normal[6]*v->ny + normal[10]*v->nz;

    w = x*x + y*y + z*z;
    w = (w>FLT_MIN) ? (1.0f / sqrt( w )) : 1.0f;

    v->nx = x * w;
    v->ny = y * w;
    v->nz = z * w;
}

void ia_draw_triangles( framebuffer* fb, void* ptr, unsigned int vertexcount )
{
    unsigned int i;
    triangle t;

    vertexcount -= vertexcount % 3;

    for( i=0; i<vertexcount; i+=3 )
    {
        ptr = read_vertex( &(t.v0), ptr );
        ptr = read_vertex( &(t.v1), ptr );
        ptr = read_vertex( &(t.v2), ptr );
        transform_vertex( &(t.v0) );
        transform_vertex( &(t.v1) );
        transform_vertex( &(t.v2) );
        triangle_rasterize( &t, fb );
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
    triangle t;

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
        transform_vertex( &(t.v0) );
        transform_vertex( &(t.v1) );
        transform_vertex( &(t.v2) );
        triangle_rasterize( &t, fb );
    }
}

