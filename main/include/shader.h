/**
 * \file shader.h
 *
 * \brief Contians shader functions
 */
#ifndef SHADER_H
#define SHADER_H



#include "predef.h"




/**
 * \enum SHADER_PROGRAM
 *
 * \brief Shader program enumerator
 */
typedef enum
{
    /**
     * \brief Do not do any lighting calulation, simply use interpolated
     *        vertex colors and apply texture colors
     */
    SHADER_UNLIT = 0,

    /**
     * \brief Calculate lighting for one vertex and use values for
     *        entire triangle
     */
    SHADER_FLAT = 1,

    /**
     * \brief Calculate lighting for every vertex and interpolate inside
     *        the triangle
     */
    SHADER_GOURAUD = 2,

    /**
     * \brief Interpolate lighting parameters (e.g. normal) and calculate
     *        lighting per fragment
     */
    SHADER_PHONG = 3
}
SHADER_PROGRAM;



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Run the vertex shader on a vertex
 *
 * \param ctx  A pointer to a context
 * \param vert A pointer to a vertex to process
 */
void shader_process_vertex( const context* ctx, rs_vertex* vert );

/**
 * \brief Run the geometry shader on a triangle
 *
 * \param ctx A pointer to a context
 * \param v0  A pointer to the first vertex of a triangle
 * \param v1  A pointer to the second vertex of a triangle
 * \param v2  A pointer to the third vertex of a triangle
 */
void shader_process_triangle( const context* ctx,
                              rs_vertex* v0, rs_vertex* v1, rs_vertex* v2 );

/**
 * \brief Run the frament shader on the interpolated vertex attributes
 *
 * \param ctx  A pointer to a context
 * \param frag A pointer to a the interpolated vertex attributes
 *
 * \return A color value for the fragment
 */
vec4 shader_process_fragment( const context* ctx, const rs_vertex* frag );

#ifdef __cplusplus
}
#endif

#endif /* SHADER_H */

