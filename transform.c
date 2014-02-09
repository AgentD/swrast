#include "transform.h"



static void vertex_transform( const vertex* in, vertex* out,
                              const float* m )
{
    float x, y, z, w;

    /* cache input position as output may point to input */
    x = in->x;
    y = in->y;
    z = in->z;
    w = in->w;

    /* multiply input position with matrix, store in output position */
    out->x = m[0]*x + m[4]*y + m[ 8]*z + m[12]*w;
    out->y = m[1]*x + m[5]*y + m[ 9]*z + m[13]*w;
    out->z = m[2]*x + m[6]*y + m[10]*z + m[14]*w;
    out->w = m[3]*x + m[7]*y + m[11]*z + m[15]*w;

    /* copy color */
    out->r = in->r;
    out->g = in->g;
    out->b = in->b;
    out->a = in->a;

    /* copy texture coordinates */
    out->s = in->s;
    out->t = in->t;
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

