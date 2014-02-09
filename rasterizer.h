#ifndef RASTERIZER_H
#define RASTERIZER_H



#include "framebuffer.h"
#include "texture.h"



#define MAX_TEXTURES 1



typedef struct
{
    float x, y, z, w;
    float r, g, b, a;
    float s, t;
}
vertex;

typedef struct
{
    vertex v0;
    vertex v1;
    vertex v2;
}
triangle;

typedef struct
{
    /** \brief Non-zero to enable depth testing, zero to disable */
    int depth_test;

    /** \brief Non-zero to enable alpha blending, zero to disable */
    int alpha_blend;

    /** \brief Non-zero to enable a texture layer, zero to disable */
    int texture_enable[MAX_TEXTURES];

    /** \brief Pointer to textures for different texture layers */
    texture* textures[MAX_TEXTURES];
}
rasterizer_state;



/**
 * \brief Overwrite the current rasterizer state
 *
 * \param state A pointer to a state structure
 */
void rasterizer_set_state( const rasterizer_state* state );

/**
 * \brief Read the current rasterizer state
 *
 * \param state A pointer to a state structure to write the current state to
 */
void rasterizer_get_state( rasterizer_state* state );

/**
 * \brief Rasterize a triangle
 *
 * \param t  A pointer to a triangle structure
 * \param fb A pointer to a frame buffer structure
 */
void triangle_rasterize( const triangle* t, framebuffer* fb );



#endif /* RASTERIZER_H */

