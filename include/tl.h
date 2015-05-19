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
 * \brief Transform a vertex of a triangle and compute lighting values
 *
 * First, the vertex is transformed to view space using the modelview matrix
 * of the context for the positon and normal matrix for the normal. Then,
 * if the context has lighting enabled, lighting is computed for all
 * configured light sources. In the last step, the vertex position is
 * transformed using the projection matrix of the context.
 *
 * \param ctx                A pointer to a context
 * \param v                  A pointer to a vertex
 * \param light_off_override If non-zero lighting is not computed even if
 *                           the context has lighting enabled
 */
void tl_transform_and_light_vertex( context* ctx, rs_vertex* v,
                                    int light_off_override );

#ifdef __cplusplus
}
#endif

#endif /* TL_H */

