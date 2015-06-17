/**
 * \file shader.h
 *
 * \brief Contians shader functions
 */
#ifndef SHADER_H
#define SHADER_H



#include "predef.h"



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Apply only transformation to a vertex
 *
 * \param ctx  A pointer to a context
 * \param vert A pointer to a vertex to process
 */
void shader_ftransform( context* ctx, rs_vertex* vert );

/**
 * \brief Run the vertex shader on a vertex
 *
 * \param ctx  A pointer to a context
 * \param vert A pointer to a vertex to process
 */
void shader_process_vertex( context* ctx, rs_vertex* vert );

/**
 * \brief Run the frament shader on the interpolated vertex attributes
 *
 * \param ctx  A pointer to a context
 * \param frag A pointer to a the interpolated vertex attributes
 *
 * \return A color value for the fragment
 */
vec4 shader_process_fragment( context* ctx, rs_vertex* frag );

#ifdef __cplusplus
}
#endif

#endif /* SHADER_H */

