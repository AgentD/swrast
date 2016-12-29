/**
 * \file inputassembler.h
 *
 * \brief Contains the rasterizer input assembler stage interface
 */
#ifndef INPUT_ASSEMBLER_H
#define INPUT_ASSEMBLER_H

#include "predef.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Read vertices from the currently set buffer and send them down the
 *        rendering pipeline
 *
 * \param ctx         A pointer to a context object
 * \param vertexcount The number of vertices to read from the input stream
 */
void ia_draw_triangles(context *ctx, unsigned int vertexcount);

/**
 * \brief Read vertices from the currently vertex & index buffers and send
 *        them down the rendering pipeline
 *
 * \param ctx         A pointer to a context object
 * \param vertexcount The number of vertices available (for bounds checking)
 * \param indexcount  The number of indices to read from the index buffer
 */
void ia_draw_triangles_indexed(context *ctx, unsigned int vertexcount,
				unsigned int indexcount);

/**
 * \brief Begin immediate mode rendering
 *
 * In immediate mode rendering, vertices and vertex attributes can be
 * specified via the functions ia_vertex, ia_color, ia_normal and
 * ia_texcoord. The vertex attributes are stored in the context and used
 * for every vertex specified via ia_vertex until they are changed. When
 * enoguh vertices are specified to draw a prititive, the primitive is
 * rendered.
 *
 * \param ctx A pointer to a context object
 */
void ia_begin(context *ctx);

/**
 * \brief Add a vertex with the previous specified color/normal/texcoord
 *
 * \param ctx A pointer to a context object
 */
void ia_vertex(context *ctx, float x, float y, float z, float w);

/**
 * \brief Specify the color attribute to use for all future vertices
 *
 * \param ctx A pointer to a context object
 */
void ia_color(context *ctx, float r, float g, float b, float a);

/**
 * \brief Specify the normal attribute to use for all future vertices
 *
 * \param ctx A pointer to a context object
 */
void ia_normal(context *ctx, float x, float y, float z);

/**
 * \brief Specify a texture coordinate to use for all future vertices
 *
 * \param ctx   A pointer to a context object
 * \param layer The texture layer index to specify the coordinate for
 */
void ia_texcoord(context *ctx, int layer, float s, float t);

/**
 * \brief Exit immediate mode rendering
 *
 * \param ctx A pointer to a context object
 */
void ia_end(context *ctx);

#ifdef __cplusplus
}
#endif

#endif /* INPUT_ASSEMBLER_H */

