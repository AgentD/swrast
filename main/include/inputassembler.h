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
void ia_draw_triangles( context* ctx, unsigned int vertexcount );

/**
 * \brief Read vertices from the currently vertex & index buffers and send
 *        them down the rendering pipeline
 *
 * \param ctx         A pointer to a context object to
 * \param vertexcount The number of vertices available (for bounds checking)
 * \param indexcount  The number of indices to read from the index buffer
 */
void ia_draw_triangles_indexed( context* ctx, unsigned int vertexcount,
                                              unsigned int indexcount );

#ifdef __cplusplus
}
#endif

#endif /* INPUT_ASSEMBLER_H */

