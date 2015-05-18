/**
 * \file tl.h
 *
 * \brief Contains the fixed function transform & light stage interface
 */
#ifndef TL_H
#define TL_H



#include "predef.h"



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Set the currently active model view matrix
 *
 * \param ctx A pointer to a context
 * \param f   A pointer to an array of 16 float values, representing a 4x4
 *            matrix, stored in column-major order
 */
void tl_set_modelview_matrix( context* ctx, float* f );

/**
 * \brief Set the currently active projection matrix
 *
 * \param ctx A pointer to a context
 * \param f   A pointer to an array of 16 float values, representing a 4x4
 *            matrix, stored in column-major order
 */
void tl_set_projection_matrix( context* ctx, float* f );

/**
 * \brief Transform the vertices of a triangle and compute lighting values
 *
 * \param ctx A pointer to a context
 * \param v0  A pointer to the first vertex of a triangle
 * \param v1  A pointer to the second vertex of a triangle
 * \param v2  A pointer to the third vertex of a triangle
 */
void tl_transform_and_light_triangle( context* ctx, rs_vertex* v0,
                                      rs_vertex* v1, rs_vertex* v2 );

#ifdef __cplusplus
}
#endif

#endif /* TL_H */

