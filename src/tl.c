#include "tl.h"
#include "context.h"
#include "rasterizer.h"

#include <math.h>



void tl_light_vertex( context* ctx, rs_vertex* v )
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

void tl_transform_vertex( context* ctx, rs_vertex* v )
{
    float *normal = ctx->normalmatrix, *mv = ctx->modelview, x, y, z, w;

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
}

void tl_project_vertex( context* ctx, rs_vertex* v )
{
    float x = v->x, y = v->y, z = v->z, w = v->w;
    float* proj = ctx->projection;

    v->x = proj[0]*x + proj[4]*y + proj[ 8]*z + proj[12]*w;
    v->y = proj[1]*x + proj[5]*y + proj[ 9]*z + proj[13]*w;
    v->z = proj[2]*x + proj[6]*y + proj[10]*z + proj[14]*w;
    v->w = proj[3]*x + proj[7]*y + proj[11]*z + proj[15]*w;
}

void tl_transform_and_light( context* ctx, rs_vertex* v0, rs_vertex* v1,
                             rs_vertex* v2 )
{
    float u[4], v[4], s;
    rs_vertex p;

    tl_transform_vertex( ctx, v0 );
    tl_transform_vertex( ctx, v1 );
    tl_transform_vertex( ctx, v2 );

    if( ctx->light_enable )
    {
        if( ctx->shade_model==SHADE_FLAT ||
            !(ctx->vertex_format & VF_NORMAL_F3) )
        {
            /* compute normal for entire triangle */
            u[0] = v1->x - v0->x; u[1] = v1->y - v0->y; u[2] = v1->z - v0->z;
            v[0] = v2->x - v0->x; v[1] = v2->y - v0->y; v[2] = v2->z - v0->z;

            p.nx = u[1]*v[2] - u[2]*v[1];
            p.ny = u[2]*v[0] - u[0]*v[2];
            p.nz = u[0]*v[1] - u[1]*v[0];
            s = p.nx*p.nx + p.ny*p.ny + p.nz*p.nz;

            if( !ctx->front_is_ccw )
                s = -s;

            s = s<0.0f ? 0.0f : (1.0f/sqrt( s ));
            p.nx *= s;
            p.ny *= s;
            p.nz *= s;

            /* set normal for every vertex */
            v0->nx = v1->nx = v2->nx = p.nx;
            v0->ny = v1->ny = v2->ny = p.ny;
            v0->nz = v1->nz = v2->nz = p.nz;
        }

        if( ctx->shade_model==SHADE_GOURAUD )
        {
            tl_light_vertex( ctx, v0 );
            tl_light_vertex( ctx, v1 );
            tl_light_vertex( ctx, v2 );
        }
        else if( ctx->shade_model==SHADE_FLAT )
        {
            /* get position of provoking vertex */
            switch( ctx->provoking_vertex )
            {
            case 0: p.x=v0->x; p.y=v0->y; p.z=v0->z; p.w=v0->w; break;
            case 1: p.x=v1->x; p.y=v1->y; p.z=v1->z; p.w=v1->w; break;
            case 2: p.x=v2->x; p.y=v2->y; p.z=v2->z; p.w=v2->w; break;
            }

            /* compute lighting for provoking vertex */
            p.r = p.g = p.b = p.a = 0xFF;
            tl_light_vertex( ctx, &p );

            /* apply lighting */
            v0->r = (v0->r*p.r)>>8;
            v0->g = (v0->g*p.g)>>8;
            v0->b = (v0->b*p.b)>>8;
            v0->a = (v0->a*p.a)>>8;

            v1->r = (v1->r*p.r)>>8;
            v1->g = (v1->g*p.g)>>8;
            v1->b = (v1->b*p.b)>>8;
            v1->a = (v1->a*p.a)>>8;

            v2->r = (v2->r*p.r)>>8;
            v2->g = (v2->g*p.g)>>8;
            v2->b = (v2->b*p.b)>>8;
            v2->a = (v2->a*p.a)>>8;
        }
    }

    tl_project_vertex( ctx, v0 );
    tl_project_vertex( ctx, v1 );
    tl_project_vertex( ctx, v2 );
}

