#include "tl.h"
#include "context.h"
#include "rasterizer.h"

#include <float.h>
#include <math.h>



static void recompute_normal_matrix( context* ctx )
{
    float *mv, *normal;
    float det, m[16];

    mv = ctx->modelview;
    normal = ctx->normalmatrix;

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

void tl_set_modelview_matrix( context* ctx, float* f )
{
    int i;

    for( i=0; i<16; ++i )
        ctx->modelview[ i ] = f[ i ];

    recompute_normal_matrix( ctx );
}

void tl_set_projection_matrix( context* ctx, float* f )
{
    int i;

    for( i=0; i<16; ++i )
        ctx->projection[ i ] = f[ i ];
}

/****************************************************************************
 *                      triangle processing functions                       *
 ****************************************************************************/

static void light_vertex( context* ctx, rs_vertex* v )
{
    float color[3] = { 0.0f, 0.0f, 0.0f };
    float L[3], V[3], H[3], d, k, a;
    int i;

    for( i=0; i<MAX_LIGHTS; ++i )
    {
        if( !ctx->light[ i ].enable )
            continue;

        /* compute light vector and distance */
        L[0] = ctx->light[i].position[0] - v->x;
        L[1] = ctx->light[i].position[1] - v->y;
        L[2] = ctx->light[i].position[2] - v->z;
        d = sqrt( L[0]*L[0] + L[1]*L[1] + L[2]*L[2] );
        k = 1.0f / d;
        L[0] *= k;
        L[1] *= k;
        L[2] *= k;

        /* compute view vector */
        V[0] = -(v->x);
        V[1] = -(v->y);
        V[2] = -(v->z);
        k = 1.0f / sqrt( V[0]*V[0] + V[1]*V[1] + V[2]*V[2] );
        V[0] *= k;
        V[1] *= k;
        V[2] *= k;

        /* compute half vector */
        H[0] = L[0] + V[0];
        H[1] = L[1] + V[1];
        H[2] = L[2] + V[2];
        k = 1.0f / sqrt( H[0]*H[0] + H[1]*H[1] + H[2]*H[2] );
        H[0] *= k;
        H[1] *= k;
        H[2] *= k;

        /* attenuation factor */
        a = 1.0f / (ctx->light[i].attenuation_constant +
                    ctx->light[i].attenuation_linear * d + 
                    ctx->light[i].attenuation_quadratic * d * d);

        /* emissive component */
        color[0] += ctx->material.emission[0];
        color[1] += ctx->material.emission[1];
        color[2] += ctx->material.emission[2];

        /* ambient component */
        color[0] += ctx->light[i].ambient[0]*ctx->material.ambient[0];
        color[1] += ctx->light[i].ambient[1]*ctx->material.ambient[1];
        color[2] += ctx->light[i].ambient[2]*ctx->material.ambient[2];

        /* diffuse component */
        d = (v->nx*L[0] + v->ny*L[1] + v->nz*L[2]) * a;
        d = d<0.0f ? 0.0f : (d>1.0f ? 1.0f : d);

        color[0]+=ctx->light[i].diffuse[0]*ctx->material.diffuse[0]*d;
        color[1]+=ctx->light[i].diffuse[1]*ctx->material.diffuse[1]*d;
        color[2]+=ctx->light[i].diffuse[2]*ctx->material.diffuse[2]*d;

        /* specular component */
        d = v->nx*H[0] + v->ny*H[1] + v->nz*H[2];
        d = d<0.0f ? 0.0f : (d>1.0f ? 1.0f : d);
        d = pow( d, ctx->material.shininess );

        color[0] += ctx->light[i].specular[0]*ctx->material.specular[0]*d;
        color[1] += ctx->light[i].specular[1]*ctx->material.specular[1]*d;
        color[2] += ctx->light[i].specular[2]*ctx->material.specular[2]*d;
    }

    v->r = color[0]*v->r;
    v->g = color[1]*v->g;
    v->b = color[2]*v->b;
}

static void process_vertex( context* ctx, rs_vertex* v )
{
    float *normal = ctx->normalmatrix, *mv = ctx->modelview;
    float *proj = ctx->projection, x, y, z, w;

    /* transform normal to viewspace */
    x = normal[0]*v->nx + normal[4]*v->ny + normal[ 8]*v->nz;
    y = normal[1]*v->nx + normal[5]*v->ny + normal[ 9]*v->nz;
    z = normal[2]*v->nx + normal[6]*v->ny + normal[10]*v->nz;

    w = x*x + y*y + z*z;
    w = (w>0.0) ? (1.0f / sqrt( w )) : 1.0f;

    v->nx = x * w;
    v->ny = y * w;
    v->nz = z * w;

    /* transform position to viewspace */
    x = mv[0]*v->x + mv[4]*v->y + mv[ 8]*v->z + mv[12]*v->w;
    y = mv[1]*v->x + mv[5]*v->y + mv[ 9]*v->z + mv[13]*v->w;
    z = mv[2]*v->x + mv[6]*v->y + mv[10]*v->z + mv[14]*v->w;
    w = mv[3]*v->x + mv[7]*v->y + mv[11]*v->z + mv[15]*v->w;

    v->x = x;
    v->y = y;
    v->z = z;
    v->w = w;

    /* compute vertex lighting */
    if( ctx->light_enable )
        light_vertex( ctx, v );

    /* project position */
    v->x = proj[0]*x + proj[4]*y + proj[ 8]*z + proj[12]*w;
    v->y = proj[1]*x + proj[5]*y + proj[ 9]*z + proj[13]*w;
    v->z = proj[2]*x + proj[6]*y + proj[10]*z + proj[14]*w;
    v->w = proj[3]*x + proj[7]*y + proj[11]*z + proj[15]*w;
}

void tl_transform_and_light_triangle( context* ctx, rs_vertex* v0,
                                      rs_vertex* v1, rs_vertex* v2 )
{
    process_vertex( ctx, v0 );
    process_vertex( ctx, v1 );
    process_vertex( ctx, v2 );
}

