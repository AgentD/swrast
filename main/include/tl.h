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
 * \brief Compute the lighting values of a vertex
 *
 * \param ctx A pointer to a context to get the lighting configuration from
 * \param v   A pointer to a vertex to compute lighting for
 */
void tl_light_vertex( context* ctx, rs_vertex* v );

/**
 * \brief Apply model-view transformation to a vertex
 *
 * The modelview matrix is used for positions and the normal matrix for
 * normals.
 *
 * \param ctx A pointer to a context to get the matrices from
 * \param v   A pointer to a vertex to transform
 */
void tl_transform_vertex( context* ctx, rs_vertex* v );

/**
 * \brief Apply projective transformation to a vertex
 *
 * \param ctx A pointer to a context to get the projection matrix from
 * \param v   A pointer to a vertex to transform
 */
void tl_project_vertex( context* ctx, rs_vertex* v );

/**
 * \brief Transform the vertices of a triangle and compute lighting values
 *
 * First, the vertices are transformed to view space using the modelview
 * matrix of the context for the positon and normal matrix for the normal.
 * Then, depending on the shade model and lighting parameters of the context,
 * the lighting values are calculated for the vertices. In the last step, the
 * vertex positions are transformed using the projection matrix of the
 * context.
 *
 * If flat shading is used, or the vertex format of the context does not have
 * surface normals, the normal of the entire triangle is computed and set for
 * every vertex.
 *
 * \param ctx A pointer to a context
 * \param v0  A pointer to the first vertex
 * \param v1  A pointer to the second vertex
 * \param v2  A pointer to the third vertex
 */
void tl_transform_and_light( context* ctx, rs_vertex* v0, rs_vertex* v1,
                             rs_vertex* v2 );

#ifdef __cplusplus
}
#endif

#endif /* TL_H */

