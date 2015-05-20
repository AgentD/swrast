#include "framebuffer.h"
#include "context.h"
#include <stddef.h>
#include <float.h>



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

    ctx->shade_model = SHADE_GOURAUD;
    ctx->provoking_vertex = 0;

    ctx->depth_test = COMPARE_ALWAYS;
    ctx->depth_near = 0.0f;
    ctx->depth_far = 1.0f;

    ctx->viewport.x = 0;
    ctx->viewport.y = 0;
    ctx->viewport.width = 0;
    ctx->viewport.height = 0;
    ctx->vertex_format = 0;
    ctx->vertexbuffer = NULL;
    ctx->indexbuffer = NULL;
    ctx->target = NULL;

    ctx->flags = DEPTH_CLIP|DEPTH_WRITE|
                 WRITE_RED|WRITE_GREEN|WRITE_BLUE|WRITE_ALPHA|
                 FRONT_CCW;
}

void context_set_modelview_matrix( context* ctx, float* f )
{
    int i;

    for( i=0; i<16; ++i )
        ctx->modelview[ i ] = f[ i ];

    recompute_normal_matrix( ctx );
}

void context_set_projection_matrix( context* ctx, float* f )
{
    int i;

    for( i=0; i<16; ++i )
        ctx->projection[ i ] = f[ i ];
}

void context_set_viewport( context* ctx, int x, int y,
                           unsigned int width, unsigned int height )
{
    ctx->viewport.x = x;
    ctx->viewport.y = y;
    ctx->viewport.width = width;
    ctx->viewport.height = height;

    ctx->draw_area.minx = x<0 ? 0 : x;
    ctx->draw_area.miny = y<0 ? 0 : y;
    ctx->draw_area.maxx = x + width - 1;
    ctx->draw_area.maxy = y + height - 1;

    if( ctx->draw_area.maxx < 0 )
        ctx->draw_area.maxx = 0;
    if( ctx->draw_area.maxy < 0 )
        ctx->draw_area.maxy = 0;

    if( ctx->draw_area.maxx >= (int)ctx->target->width )
        ctx->draw_area.maxx = ctx->target->width - 1;
    if( ctx->draw_area.maxy >= (int)ctx->target->height )
        ctx->draw_area.maxy = ctx->target->height - 1;
    if( ctx->draw_area.minx >= (int)ctx->target->width )
        ctx->draw_area.minx = ctx->target->width - 1;
    if( ctx->draw_area.miny >= (int)ctx->target->height )
        ctx->draw_area.miny = ctx->target->height - 1;
}

