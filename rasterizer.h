#ifndef RASTERIZER_H
#define RASTERIZER_H



#include "framebuffer.h"



#define MAX_TEXTURES 1



typedef struct
{
    float x, y, z, w;
    float nx, ny, nz;
    float s[MAX_TEXTURES], t[MAX_TEXTURES];
    unsigned char r, g, b, a;
}
vertex;

typedef struct
{
    float w;
    float s[MAX_TEXTURES], t[MAX_TEXTURES];
    unsigned char r, g, b, a;
    int d;
}
rs_vertex;

typedef struct
{
    vertex v0;
    vertex v1;
    vertex v2;
}
triangle;

typedef struct
{
    /** \brief Non-zero to cull counter clockwise triangles */
    int cull_ccw;

    /** \brief Non-zero to cull clockwise triangles */
    int cull_cw;
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
 * This function rasterizes to a frame buffer after applying all
 * transformations to the triangle but BEFORE perspective division.
 * The function does perspective divsion internally. The w coordinate
 * before perpective division is needed for perspective correct
 * interpolation.
 *
 * \param t  A pointer to a triangle structure
 * \param fb A pointer to a frame buffer structure
 */
void triangle_rasterize( const triangle* t, framebuffer* fb );



#endif /* RASTERIZER_H */

