#include "tl.h"
#include "context.h"
#include "rasterizer.h"

#include <math.h>



void tl_light_vertex( context* ctx, rs_vertex* v )
{
    vec4 color = { 0.0f, 0.0f, 0.0f, 1.0f }, L, V, H, ca, cd, cs;
    float d, a;
    int i;

    for( i=0; i<MAX_LIGHTS; ++i )
    {
        if( !ctx->light[ i ].enable )
            continue;

        /* compute light vector and distance */
        vec4_diff( &L, &ctx->light[i].position, &v->pos );
        L.w = 0.0f;
        d = sqrt( vec4_dot( &L, &L ) );
        vec4_scale( &L, d>0.0f ? 1.0f/d : 0.0f );

        /* compute view vector */
        vec4_get_inverted( &V, &v->pos );
        V.w = 0.0f;
        vec4_normalize( &V );

        /* compute half vector */
        vec4_sum( &H, &L, &V );
        vec4_normalize( &H );

        /* attenuation factor */
        a = 1.0f / (ctx->light[i].attenuation_constant +
                    ctx->light[i].attenuation_linear * d + 
                    ctx->light[i].attenuation_quadratic * d * d);

        /* ambient component */
        vec4_product( &ca, &ctx->light[i].ambient, &ctx->material.ambient );

        /* diffuse component */
        d = vec4_dot( &v->normal, &L );
        d = d<0.0f ? 0.0f : (d>1.0f ? 1.0f : d);

        vec4_product( &cd, &ctx->light[i].diffuse, &ctx->material.diffuse );
        vec4_scale( &cd, d * a );

        /* specular component */
        d = vec4_dot( &v->normal, &H );
        d = d<0.0f ? 0.0f : (d>1.0f ? 1.0f : d);
        d = pow( d, ctx->material.shininess );

        vec4_product( &cs, &ctx->light[i].specular, &ctx->material.specular );
        vec4_scale( &cs, d * a );

        vec4_add( &color, &ctx->material.emission );
        vec4_add( &color, &ca );
        vec4_add( &color, &cd );
        vec4_add( &color, &cs );
    }

    vec4_mul( &v->color, &color );
}

void tl_transform_vertex( context* ctx, rs_vertex* v )
{
    /* transform normal to viewspace */
    vec4_transform( &v->normal, ctx->normalmatrix, &v->normal );
    v->normal.w = 0.0f;
    vec4_normalize( &v->normal );

    /* transform position to viewspace */
    vec4_transform( &v->pos, ctx->modelview, &v->pos );
}

void tl_project_vertex( context* ctx, rs_vertex* v )
{
    vec4_transform( &v->pos, ctx->projection, &v->pos );
}

void tl_transform_and_light( context* ctx, rs_vertex* v0, rs_vertex* v1,
                             rs_vertex* v2 )
{
    rs_vertex p;
    vec4 u, v;
    float s;

    tl_transform_vertex( ctx, v0 );
    tl_transform_vertex( ctx, v1 );
    tl_transform_vertex( ctx, v2 );

    if( ctx->flags & LIGHT_ENABLE )
    {
        if( ctx->shade_model==SHADE_FLAT ||
            !(ctx->vertex_format & VF_NORMAL_F3) )
        {
            /* compute normal for entire triangle */
            vec4_diff( &u, &v1->pos, &v0->pos );
            vec4_diff( &v, &v2->pos, &v0->pos );

            vec4_cross( &p.normal, &u, &v, 0.0f );
            s = vec4_dot( &p.normal, &p.normal );

            if( !(ctx->flags & FRONT_CCW) )
                s = -s;

            s = s<0.0f ? 0.0f : (1.0f/sqrt( s ));
            vec4_scale( &p.normal, s );

            /* set normal for every vertex */
            v0->normal = p.normal;
            v1->normal = p.normal;
            v2->normal = p.normal;
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
            case 0: p.pos = v0->pos; break;
            case 1: p.pos = v1->pos; break;
            case 2: p.pos = v2->pos; break;
            }

            /* compute lighting for provoking vertex */
            vec4_set( &p.color, 1.0f, 1.0f, 1.0f, 1.0f );
            tl_light_vertex( ctx, &p );

            /* apply lighting */
            vec4_mul( &v0->color, &p.color );
            vec4_mul( &v1->color, &p.color );
            vec4_mul( &v2->color, &p.color );
        }
    }

    tl_project_vertex( ctx, v0 );
    tl_project_vertex( ctx, v1 );
    tl_project_vertex( ctx, v2 );
}

