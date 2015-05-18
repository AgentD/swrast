#include "context.h"
#include <stddef.h>



void context_init( context* ctx )
{
    int i, j;

    for( i=0; i<MAX_LIGHTS; ++i )
    {
        for( j=0; j<4; ++j )
        {
            ctx->light[i].ambient[j]  = j==3 ? 1.0f : 0.0f;
            ctx->light[i].diffuse[j]  = i==0 ? 1.0f : 0.0f;
            ctx->light[i].specular[j] = i==0 ? 1.0f : 0.0f;
            ctx->light[i].position[j] = j==3 ? 1.0f : 0.0f;
        }

        ctx->light[i].attenuation_constant = 0.0f;
        ctx->light[i].attenuation_linear = 0.0f;
        ctx->light[i].attenuation_quadratic = 0.0f;
        ctx->light[i].enable = 0;
    }

    for( i=0; i<4; ++i )
    {
        ctx->material.ambient[i] = 1.0f;
        ctx->material.diffuse[i] = 1.0f;
        ctx->material.specular[i] = 1.0f;
        ctx->material.emission[i] = i==3 ? 1.0f : 0.0f;
    }

    ctx->material.shininess = 0;

    ctx->light_enable = 0;
    ctx->cull_ccw = 0;
    ctx->cull_cw = 0;
    ctx->depth_test = COMPARE_ALWAYS;
    ctx->alpha_blend = 0;

    for( i=0; i<MAX_TEXTURES; ++i )
    {
        ctx->texture_enable[i] = 0;
        ctx->textures[i] = NULL;
    }

    for( i=0; i<16; ++i )
        ctx->modelview[i] = ctx->projection[i] = ctx->normalmatrix[i] = 0.0f;

    ctx->projection[ 0] = ctx->modelview[ 0] = ctx->normalmatrix[ 0] = 1.0f;
    ctx->projection[ 5] = ctx->modelview[ 5] = ctx->normalmatrix[ 5] = 1.0f;
    ctx->projection[10] = ctx->modelview[10] = ctx->normalmatrix[10] = 1.0f;
    ctx->projection[15] = ctx->modelview[15] = ctx->normalmatrix[15] = 1.0f;

    ctx->vertex_format = 0;
    ctx->vertexbuffer = NULL;
    ctx->indexbuffer = NULL;
    ctx->target = NULL;
}

