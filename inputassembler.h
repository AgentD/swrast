#ifndef INPUT_ASSEMBLER_H
#define INPUT_ASSEMBLER_H



#include "framebuffer.h"



typedef enum
{
    VF_POSITION_F2 = 0x0001,    /**< \brief 2 component float position */
    VF_POSITION_F3 = 0x0002,    /**< \brief 3 component float position */
    VF_POSITION_F4 = 0x0004,    /**< \brief 4 component float position */

    VF_NORMAL_F3 = 0x0010,      /**< \brief 3 component float normal */

    VF_COLOR_F3 = 0x0100,       /**< \brief 3 component float color */
    VF_COLOR_F4 = 0x0200,       /**< \brief 4 component float color */
    VF_COLOR_UB3 = 0x0400,      /**< \brief 3 component unsigned byte color */
    VF_COLOR_UB4 = 0x0800,      /**< \brief 4 component unsigned byte color */

    /**
     * \brief 2 component float texture coordinates for texture channel 0
     */
    VF_TEX0 = 0x1000
}
VERTEX_FORMAT;



/**
 * \brief Set the currently active model view matrix
 *
 * \param f A pointer to an array of 16 float values, representing a 4x4
 *          matrix, stored in column-major order
 */
void ia_set_modelview_matrix( float* f );

/**
 * \brief Set the currently active projection matrix
 *
 * \param f A pointer to an array of 16 float values, representing a 4x4
 *          matrix, stored in column-major order
 */
void ia_set_projection_matrix( float* f );

/**
 * \brief Set the vertex format for the next vertex stream
 *
 * \param format A combination of VERTEX_FORMAT flags
 */
void ia_set_vertex_format( int format );

/**
 * \brief Draw a stream of triangles using the currently set vertex format
 *        and transformation matrices
 *
 * \param fb          A pointer to a framebuffer object to rasterize to
 * \param ptr         A pointer to the input vertex stream
 * ßparam vertexcount The number of vertices to read from the input stream
 */
void ia_draw_triangles( framebuffer* fb, void* ptr,
                        unsigned int vertexcount );

/**
 * \brief Draw an indexed stream of triangles using the currently set vertex
 *        format and transformation matrices
 *
 * \param fb          A pointer to a framebuffer object to rasterize to
 * \param ptr         A pointer to the input vertex stream
 * \param vertexcount The number of vertices available (for bounds checking)
 * \param indices     A pointer to the input index buffer
 * \param indexcount  The number of indices to read from the index buffer
 */
void ia_draw_triangles_indexed( framebuffer* fb, void* ptr,
                                unsigned int vertexcount,
                                unsigned short* indices,
                                unsigned int indexcount );



#endif /* INPUT_ASSEMBLER_H */

