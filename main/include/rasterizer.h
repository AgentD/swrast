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

typedef enum {
	ATTRIB_POS = 0,
	ATTRIB_COLOR = 1,
	ATTRIB_NORMAL = 2,
	ATTRIB_TEX0 = 3,
	ATTRIB_TEX1 = 4,
	ATTRIB_USR0 = 5,
	ATTRIB_USR1 = 6,

	ATTRIB_COUNT = 7
} ATTRIB_SLOT;

typedef enum {
	ATTRIB_FLAG_POS = 0x01,
	ATTRIB_FLAG_COLOR = 0x02,
	ATTRIB_FLAG_NORMAL = 0x04,
	ATTRIB_FLAG_TEX0 = 0x08,
	ATTRIB_FLAG_TEX1 = 0x10,
	ATTRIB_FLAG_USR0 = 0x20,
	ATTRIB_FLAG_USR1 = 0x40
} ATTRIB_FLAGS;

/**
 * \struct rs_vertex
 *
 * \brief Holds the data of a triangle vertex before rasterization
 */
struct rs_vertex {
	vec4 attribs[ATTRIB_COUNT];
	int used;
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
void rasterizer_process_triangle(context *ctx, const rs_vertex *v0,
				const rs_vertex *v1, const rs_vertex *v2);

#ifdef __cplusplus
}
#endif

#endif /* RASTERIZER_H */

