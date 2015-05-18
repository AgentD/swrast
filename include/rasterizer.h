/**
 * \file pixel.h
 *
 * \brief Contains the rasterizer stage interface
 */
#ifndef RASTERIZER_H
#define RASTERIZER_H



#include "predef.h"
#include "config.h"



/**
 * \struct rs_vertex
 *
 * \brief Holds the data of a triangle vertex before rasterization
 */
struct rs_vertex
{
    float x, y, z, w;
    float nx, ny, nz;
    float s[MAX_TEXTURES], t[MAX_TEXTURES];
    unsigned char r, g, b, a;
};



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Rasterize a triangle
 *
 * This function rasterizes to a frame buffer after applying all
 * transformations to the triangle but BEFORE perspective division.
 * The function does perspective divsion internally. The w coordinate
 * before perpective division is needed for perspective correct
 * interpolation.
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

