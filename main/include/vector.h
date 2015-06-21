#ifndef VECTOR_H
#define VECTOR_H



#include "predef.h"
#include <math.h>



struct vec4
{
    float x;
    float y;
    float z;
    float w;
};



static void vec4_set( vec4* v, float x, float y, float z, float w )
{
    v->x = x;
    v->y = y;
    v->z = z;
    v->w = w;
}

static void vec4_copy( vec4* v, const vec4* src )
{
    v->x = src->x;
    v->y = src->y;
    v->z = src->z;
    v->w = src->w;
}

static void vec4_mul( vec4* v, const vec4* o )
{
    v->x *= o->x;
    v->y *= o->y;
    v->z *= o->z;
    v->w *= o->w;
}

static void vec4_scale( vec4* v, float s )
{
    v->x *= s;
    v->y *= s;
    v->z *= s;
    v->w *= s;
}

static void vec4_get_scaled( vec4* v, const vec4* o, float s )
{
    v->x = o->x * s;
    v->y = o->y * s;
    v->z = o->z * s;
    v->w = o->w * s;
}

static void vec4_get_inverted( vec4* v, const vec4* o )
{
    v->x = -o->x;
    v->y = -o->y;
    v->z = -o->z;
    v->w = -o->w;
}

static void vec4_diff( vec4* v, const vec4* a, const vec4* b )
{
    v->x = a->x - b->x;
    v->y = a->y - b->y;
    v->z = a->z - b->z;
    v->w = a->w - b->w;
}

static void vec4_sum( vec4* v, const vec4* a, const vec4* b )
{
    v->x = a->x + b->x;
    v->y = a->y + b->y;
    v->z = a->z + b->z;
    v->w = a->w + b->w;
}

static void vec4_product( vec4* v, const vec4* a, const vec4* b )
{
    v->x = a->x * b->x;
    v->y = a->y * b->y;
    v->z = a->z * b->z;
    v->w = a->w * b->w;
}

static void vec4_cross( vec4* v, const vec4* a, const vec4* b, float w )
{
    v->x = a->y*b->z - a->z*b->y;
    v->y = a->z*b->x - a->x*b->z;
    v->z = a->x*b->y - a->y*b->x;
    v->w = w;
}

static void vec4_add( vec4* v, const vec4* o )
{
    v->x += o->x;
    v->y += o->y;
    v->z += o->z;
    v->w += o->w;
}

static void vec4_transform( vec4* out, const float* m, const vec4* in )
{
    float x = in->x, y = in->y, z = in->z, w = in->w;

    out->x = m[0]*x + m[4]*y + m[ 8]*z + m[12]*w;
    out->y = m[1]*x + m[5]*y + m[ 9]*z + m[13]*w;
    out->z = m[2]*x + m[6]*y + m[10]*z + m[14]*w;
    out->w = m[3]*x + m[7]*y + m[11]*z + m[15]*w;
}

static float vec4_dot( const vec4* a, const vec4* b )
{
    return a->x * b->x + a->y * b->y + a->z * b->z + a->w * b->w;
}

static void vec4_normalize( vec4* v )
{
    float s = v->x * v->x + v->y * v->y + v->z * v->z;
    s = s>0.0f ? 1.0f/sqrt( s ) : 0.0f;
    vec4_scale( v, s );
}

static void vec4_mix( vec4* v, const vec4* a, const vec4* b, float s )
{
    float inv = 1.0f - s;

    v->x = a->x * inv + b->x * s;
    v->y = a->y * inv + b->y * s;
    v->z = a->z * inv + b->z * s;
    v->w = a->w * inv + b->w * s;
}


#endif /* VECTOR_H */

