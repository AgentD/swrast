#include "transform.h"



static void vertex_transform( const vertex* in, vertex* out,
                              const float* m )
{
    /* multiply input position with matrix, store in output position */
    out->x = m[0]*in->x + m[4]*in->y + m[ 8]*in->z + m[12]*in->w;
    out->y = m[1]*in->x + m[5]*in->y + m[ 9]*in->z + m[13]*in->w;
    out->z = m[2]*in->x + m[6]*in->y + m[10]*in->z + m[14]*in->w;
    out->w = m[3]*in->x + m[7]*in->y + m[11]*in->z + m[15]*in->w;

    /* copy color */
    out->r = in->r;
    out->g = in->g;
    out->b = in->b;
    out->a = in->a;
}

/****************************************************************************/

void triangle_transform( const triangle* in, triangle* out,
                         const float* matrix )
{
    if( in && out && matrix )
    {
        vertex_transform( &(in->v0), &(out->v0), matrix );
        vertex_transform( &(in->v1), &(out->v1), matrix );
        vertex_transform( &(in->v2), &(out->v2), matrix );
    }
}

void triangle_perspective_divide( triangle* t )
{
    float iw;

    if( t )
    {
        iw = 1.0f / t->v0.w;
        t->v0.x *= iw;
        t->v0.y *= iw;
        t->v0.z *= iw;
        t->v0.w = 1.0f;

        iw = 1.0f / t->v1.w;
        t->v1.x *= iw;
        t->v1.y *= iw;
        t->v1.z *= iw;
        t->v1.w = 1.0f;

        iw = 1.0f / t->v2.w;
        t->v2.x *= iw;
        t->v2.y *= iw;
        t->v2.z *= iw;
        t->v2.w = 1.0f;
    }
}

