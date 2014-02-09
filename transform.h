#ifndef TRANSFORM_H
#define TRANSFORM_H



#include "rasterizer.h"



/**
 * \brief Transfrom a triangle
 *
 * \param in     A pointer to the input triangle structure
 * \param out    A pointer to the output triangle structure
 * \param matrix A pointer to a 4x4 matrix in column-major format
 */
void triangle_transform( const triangle* in, triangle* out,
                         const float* matrix );



#endif /* TRANSFORM_H */

