#include "tl.h"
#include "context.h"
#include "rasterizer.h"

#include <math.h>



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

