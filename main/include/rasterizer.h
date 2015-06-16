/**
 * \file rasterizer.h
 *
 * \brief Contains the rasterizer stage interface
 */
#ifndef RASTERIZER_H
#define RASTERIZER_H



#include "predef.h"
#include "config.h"
#include "vector.h"



/**
 * \struct rs_vertex
 *
 * \brief Holds the data of a triangle vertex before rasterization
 */
struct rs_vertex
{
    vec4 pos;
    vec4 normal;
    vec4 texcoord[MAX_TEXTURES];
    vec4 color;
};



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Rasterize a triangle
 *
 * This function rasterizes to a frame buffer. No transformations are applied
 * to the triangle. Perspective division is performed by the function and
 * the w coordinate is used for perspective correct interpolation of vertex
 * attributes. Shading, depth test, texturing and blending are performed by
 * the function, depending on the context state. The function draws to the
 * currenlty bound target frame buffer of the context.
 *
 * \param ctx A pointer to a context object
 * \param v0  The first vertex of the triangle
 * \param v1  The second vertex of the triangle
 * \param v2  The third vertex of the triangle
 */
void rasterizer_process_triangle( context* ctx,
                                  const rs_vertex* v0, const rs_vertex* v1,
                                  const rs_vertex* v2 );

#ifdef __cplusplus
}
#endif

#endif /* RASTERIZER_H */

