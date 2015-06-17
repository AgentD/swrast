#include "rasterizer.h"
#include "context.h"
#include "texture.h"
#include "shader.h"

#include <math.h>



static void calculate_lighting( context* ctx, rs_vertex* v )
{
    vec4 color = { 0.0f, 0.0f, 0.0f, 1.0f }, L, V, H, ca, cd, cs;
    float d, a;
    int i;

    for( i=0; i<MAX_LIGHTS; ++i )
    {
        if( !ctx->light[ i ].enable )
            continue;

        /* compute light vector and distance */
        vec4_diff( &L, &ctx->light[i].position, v->attribs+ATTRIB_POS );
        L.w = 0.0f;
        d = sqrt( vec4_dot( &L, &L ) );
        vec4_scale( &L, d>0.0f ? 1.0f/d : 0.0f );

        /* compute view vector */
        vec4_get_inverted( &V, v->attribs+ATTRIB_POS );
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
        d = vec4_dot( v->attribs+ATTRIB_NORMAL, &L );
        d = d<0.0f ? 0.0f : (d>1.0f ? 1.0f : d);

        vec4_product( &cd, &ctx->light[i].diffuse, &ctx->material.diffuse );
        vec4_scale( &cd, d * a );

        /* specular component */
        d = vec4_dot( v->attribs+ATTRIB_NORMAL, &H );
        d = d<0.0f ? 0.0f : (d>1.0f ? 1.0f : d);
        d = pow( d, ctx->material.shininess );

        vec4_product( &cs, &ctx->light[i].specular, &ctx->material.specular );
        vec4_scale( &cs, d * a );

        /* accumulate */
        vec4_add( &color, &ctx->material.emission );
        vec4_add( &color, &ca );
        vec4_add( &color, &cd );
        vec4_add( &color, &cs );
    }

    color.w = 1.0f;

    /* modulate color */
    if( v->used & ATTRIB_FLAG_COLOR )
    {
        vec4_mul( v->attribs + ATTRIB_COLOR, &color );
    }
    else
    {
        v->used |= ATTRIB_FLAG_COLOR;
        v->attribs[ ATTRIB_COLOR ] = color;
    }
}

static void mv_transform( context* ctx, rs_vertex* v )
{
    /* transform normal to viewspace */
    vec4_transform( v->attribs+ATTRIB_NORMAL, ctx->normalmatrix,
                    v->attribs+ATTRIB_NORMAL );
    v->attribs[ATTRIB_NORMAL].w = 0.0f;
    vec4_normalize( v->attribs+ATTRIB_NORMAL );

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

    if( frag->used & ATTRIB_FLAG_COLOR )
        c = frag->attribs[ATTRIB_COLOR];
    else
        vec4_set( &c, 1.0f, 1.0f, 1.0f, 1.0f );

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

