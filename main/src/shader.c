#include "rasterizer.h"
#include "context.h"
#include "texture.h"
#include "shader.h"

#include <math.h>



static void calculate_lighting( context* ctx, rs_vertex* v )
{
    vec4 color = { 0.0f, 0.0f, 0.0f, 1.0f }, L, V, H, N, ca, cd, cs;
    float dist, a;
    int i;

    for( i=0; i<MAX_LIGHTS; ++i )
    {
        if( !ctx->light[ i ].enable )
            continue;

        vec4_get_inverted( &V, v->attribs+ATTRIB_POS ); /* view vector */
        vec4_sum( &L, &ctx->light[i].position, &V );    /* light vector */
        N = v->attribs[ATTRIB_NORMAL];                  /* surface normal */
        N.w = V.w = L.w = 0.0f;

        vec4_sum( &H, &L, &V );                         /* half vector */

        dist = sqrt( vec4_dot( &L, &L ) );
        vec4_scale( &L, dist>0.0f ? 1.0f/dist : 0.0f );

        vec4_normalize( &V );
        vec4_normalize( &H );
        vec4_normalize( &N );

        /* ambient component */
        vec4_product( &ca, &ctx->light[i].ambient, &ctx->material.ambient );

        /* diffuse component */
        a = vec4_dot( &N, &L );
        a = a<0.0f ? 0.0f : a;

        vec4_product( &cd, &ctx->light[i].diffuse, &ctx->material.diffuse );
        vec4_scale( &cd, a );

        /* specular component */
        a = vec4_dot( &N, &H );
        a = a<0.0f ? 0.0f : a;
        a = pow( a, ctx->material.shininess );

        vec4_product( &cs, &ctx->light[i].specular, &ctx->material.specular );
        vec4_scale( &cs, a );

        /* attenuation factor */
        a = ctx->light[i].attenuation_constant +
            ctx->light[i].attenuation_linear * dist + 
            ctx->light[i].attenuation_quadratic * dist * dist;
        a = a>0.0f ? 1.0f/a : 0.0f;

        vec4_scale( &cd, a );
        vec4_scale( &cs, a );

        /* accumulate */
        vec4_add( &color, &ctx->material.emission );
        vec4_add( &color, &ca );
        vec4_add( &color, &cd );
        vec4_add( &color, &cs );
    }

    color.w = 1.0f;

    /* modulate color */
    vec4_mul( v->attribs + ATTRIB_COLOR, &color );
    v->used |= ATTRIB_FLAG_COLOR;
}

static void mv_transform( context* ctx, rs_vertex* v )
{
    /* transform normal to viewspace */
    vec4_transform( v->attribs+ATTRIB_NORMAL, ctx->normalmatrix,
                    v->attribs+ATTRIB_NORMAL );
    v->attribs[ATTRIB_NORMAL].w = 0.0f;

    /* transform position to viewspace */
    vec4_transform( v->attribs+ATTRIB_POS, ctx->modelview,
                    v->attribs+ATTRIB_POS );
}

/****************************************************************************/

void shader_ftransform( context* ctx, rs_vertex* vert )
{
    mv_transform( ctx, vert );

    vec4_transform( vert->attribs+ATTRIB_POS, ctx->projection,
                    vert->attribs+ATTRIB_POS );
}

void shader_process_vertex( context* ctx, rs_vertex* vert )
{
    mv_transform( ctx, vert );

    if( ctx->flags & LIGHT_ENABLE )
        calculate_lighting( ctx, vert );

    vec4_transform( vert->attribs+ATTRIB_POS, ctx->projection,
                    vert->attribs+ATTRIB_POS );
}

vec4 shader_process_fragment( context* ctx, rs_vertex* frag )
{
    vec4 c, tex;
    int i;

    c = frag->attribs[ATTRIB_COLOR];

    for( i=0; i<MAX_TEXTURES; ++i )
    {
        if( ctx->texture_enable[ i ] )
        {
            texture_sample(ctx->textures[i],frag->attribs+ATTRIB_TEX0+i,&tex);
            vec4_mul( &c, &tex );
        }
    }

    return c;
}

