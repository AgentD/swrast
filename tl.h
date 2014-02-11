#ifndef TL_H
#define TL_H



#include "rasterizer.h"



/**
 * \brief Set the currently active model view matrix
 *
 * \param f A pointer to an array of 16 float values, representing a 4x4
 *          matrix, stored in column-major order
 */
void tl_set_modelview_matrix( float* f );

/**
 * \brief Set the currently active projection matrix
 *
 * \param f A pointer to an array of 16 float values, representing a 4x4
 *          matrix, stored in column-major order
 */
void tl_set_projection_matrix( float* f );

/**
 * \brief Transform the vertices of a triangle and compute lighting values
 *
 * \param t A pointer to a triangle structure
 */
void tl_transform_and_shade_triangle( triangle* t );



#endif /* TL_H */

