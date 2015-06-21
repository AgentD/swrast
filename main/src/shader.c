#include "rasterizer.h"
#include "context.h"
#include "texture.h"
#include "shader.h"

#include <math.h>
#include <stddef.h>



static void blinn_phong( context* ctx, int i, const vec4* V, const vec4* N,
                         vec4* out )
{
    float dist, att, ks, kd;
    vec4 L, H, cd, cs;

    /* light vector */
    vec4_sum( &L, &ctx->light[i].position, V );
    L.w = 0.0f;

    dist = sqrt( vec4_dot( &L, &L ) );
    vec4_scale( &L, dist>0.0f ? 1.0f/dist : 0.0f );

    /* half vector */
    vec4_sum( &H, &L, V );
    vec4_normalize( &H );

    /* attenuation factor */
    att = ctx->light[i].attenuation_constant +
          ctx->light[i].attenuation_linear * dist +
          ctx->light[i].attenuation_quadratic * dist * dist;
    att = att>0.0f ? 1.0f/att : 0.0f;

    /* diffuse component */
    kd = vec4_dot( N, &L );
    kd = kd<0.0f ? 0.0f : kd;

    /* specular component */
    ks = vec4_dot( N, &H );
    ks = ks<0.0f ? 0.0f : ks;
    ks = pow( ks, ctx->material.shininess );

    /* combine */
    vec4_product( &cd, &ctx->light[i].diffuse, &ctx->material.diffuse );
    vec4_product( &cs, &ctx->light[i].specular, &ctx->material.specular );
    vec4_scale( &cd, kd*att );
    vec4_scale( &cs, ks*att );
    vec4_add( out, &cd );
    vec4_add( out, &cs );
}

static void calculate_lighting( context* ctx, rs_vertex* v )
{
    vec4 color = { 0.0f, 0.0f, 0.0f, 1.0f }, V, N, ca;
    int i;

    vec4_get_inverted( &V, v->attribs+ATTRIB_POS ); /* view vector */
    N = v->attribs[ATTRIB_NORMAL];                  /* surface normal */
    V.w = 0.0f;

    vec4_normalize( &V );
    vec4_normalize( &N );

    for( i=0; i<MAX_LIGHTS; ++i )
    {
        if( ctx->light[ i ].enable )
        {
            blinn_phong( ctx, i, &V, &N, &color );

            vec4_product(&ca, &ctx->light[i].ambient, &ctx->material.ambient);
            vec4_add( &color, &ca );
        }
    }

    vec4_add( &color, &ctx->material.emission );
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

    /* transform position to viewspace */
    vec4_transform( v->attribs+ATTRIB_POS, ctx->modelview,
                    v->attribs+ATTRIB_POS );
}

static void apply_textures( context* ctx, rs_vertex* frag )
{
    vec4 tex;
    int i;

    for( i=0; i<MAX_TEXTURES; ++i )
    {
        if( ctx->texture_enable[ i ] )
        {
            texture_sample(ctx->textures[i],frag->attribs+ATTRIB_TEX0+i,&tex);
            vec4_mul( frag->attribs+ATTRIB_COLOR, &tex );
        }
    }
}

/****************************************************************************/

static void shader_gouraud_vertex( context* ctx, rs_vertex* vert )
{
    mv_transform( ctx, vert );

    if( ctx->flags & LIGHT_ENABLE )
        calculate_lighting( ctx, vert );

    vec4_transform( vert->attribs+ATTRIB_POS, ctx->projection,
                    vert->attribs+ATTRIB_POS );

    vert->used &= ~ATTRIB_FLAG_NORMAL;
}

static void shader_geometry_flat( context* ctx, rs_vertex* v0, rs_vertex* v1,
                                  rs_vertex* v2 )
{
    rs_vertex* p;
    int i, j;

    /* get provoking vertex */
    switch( ctx->provoking_vertex )
    {
    case 0: p = v0; break;
    case 1: p = v1; break;
    case 2: p = v2; break;
    default:        return;
    }

    /* compute light for provoking vertex */
    if( ctx->flags & LIGHT_ENABLE )
        calculate_lighting( ctx, p );

    /* apply projection all vertices */
    vec4_transform( v0->attribs+ATTRIB_POS, ctx->projection,
                    v0->attribs+ATTRIB_POS );
    vec4_transform( v1->attribs+ATTRIB_POS, ctx->projection,
                    v1->attribs+ATTRIB_POS );
    vec4_transform( v2->attribs+ATTRIB_POS, ctx->projection,
                    v2->attribs+ATTRIB_POS );

    /* copy attributes of provoking vertex */
    p->used &= ~ATTRIB_FLAG_NORMAL;
    v0->used = v1->used = v2->used = p->used;

    for( i=0, j=0x01; i<ATTRIB_COUNT; ++i, j<<=1 )
    {
        if( (p->used & j) && i!=ATTRIB_POS )
            v0->attribs[i]=v1->attribs[i]=v2->attribs[i]=p->attribs[i];
    }
}

static vec4 shader_default_fragment( context* ctx, rs_vertex* frag )
{
    apply_textures( ctx, frag );
    return frag->attribs[ATTRIB_COLOR];
}

/****************************************************************************/

static void shader_phong_vertex( context* ctx, rs_vertex* vert )
{
    mv_transform( ctx, vert );

    vert->attribs[ ATTRIB_USR0 ] = vert->attribs[ ATTRIB_POS ];
    vert->used |= (ATTRIB_FLAG_USR0|ATTRIB_FLAG_COLOR);

    vec4_normalize( vert->attribs+ATTRIB_NORMAL );

    vec4_transform( vert->attribs+ATTRIB_POS, ctx->projection,
                    vert->attribs+ATTRIB_POS );
}

static vec4 shader_phong_fragment( context* ctx, rs_vertex* frag )
{
    if( ctx->flags & LIGHT_ENABLE )
    {
        frag->attribs[ATTRIB_POS] = frag->attribs[ATTRIB_USR0];
        vec4_normalize( frag->attribs+ATTRIB_NORMAL );
        calculate_lighting( ctx, frag );
    }

    apply_textures( ctx, frag );
    return frag->attribs[ATTRIB_COLOR];
}

/****************************************************************************/

static struct shader
{
    void(* vertex )( context* ctx, rs_vertex* vert );
    void(* geometry )( context* ctx,
                       rs_vertex* v0, rs_vertex* v1, rs_vertex* v2 );
    vec4(* fragment )( context* ctx, rs_vertex* frag );
}
shaders[ ] =
{
    { mv_transform,          shader_geometry_flat, shader_default_fragment },
    { shader_gouraud_vertex, NULL,                 shader_default_fragment },
    { shader_phong_vertex,   NULL,                 shader_phong_fragment   }
};


void shader_process_vertex( context* ctx, rs_vertex* vert )
{
    shaders[ ctx->shader ].vertex( ctx, vert );
}

void shader_process_triangle( context* ctx,
                              rs_vertex* v0, rs_vertex* v1, rs_vertex* v2 )
{
    struct shader* s = shaders + ctx->shader;

    if( s->geometry )
        s->geometry( ctx, v0, v1, v2 );
}

vec4 shader_process_fragment( context* ctx, rs_vertex* frag )
{
    return shaders[ ctx->shader ].fragment( ctx, frag );
}

