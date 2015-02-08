/**
 * \file pixel.h
 *
 * \brief Contains the rasterizer stage interface
 */
#ifndef RASTERIZER_H
#define RASTERIZER_H



#include "framebuffer.h"



#define MAX_TEXTURES 1



/**
 * \struct vertex
 *
 * \brief Holds the data of a triangle vertex before rasterization
 */
typedef struct
{
    float x, y, z, w;
    float nx, ny, nz;
    float s[MAX_TEXTURES], t[MAX_TEXTURES];
    unsigned char r, g, b, a;
}
rs_vertex;

/**
 * \struct rs_fragment
 *
 * \brief The data of a fragment of a triangle with interpolated vertex values
 */
typedef struct
{
    float w;
    float s[MAX_TEXTURES], t[MAX_TEXTURES];
    unsigned char r, g, b, a;
    int d;
}
rs_fragment;

/**
 * \struct rs_triangle
 *
 * \brief Holds the data of a triangle before rasterization
 */
typedef struct
{
    rs_vertex v0;   /**< \brief First vertex */
    rs_vertex v1;   /**< \brief Second vertex */
    rs_vertex v2;   /**< \brief Third vertex */
}
rs_triangle;

/**
 * \struct rs_state
 *
 * \brief Holds the state of the rasterizer
 */
typedef struct
{
    /** \brief Non-zero to cull counter clockwise triangles */
    int cull_ccw;

    /** \brief Non-zero to cull clockwise triangles */
    int cull_cw;
}
rs_state;



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Overwrite the current rasterizer state
 *
 * \param state A pointer to a state structure
 */
void rasterizer_set_state( const rs_state* state );

/**
 * \brief Read the current rasterizer state
 *
 * \param state A pointer to a state structure to write the current state to
 */
void rasterizer_get_state( rs_state* state );

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
void rasterizer_process_triangle( const rs_triangle* t, framebuffer* fb );

#ifdef __cplusplus
}
#endif

#endif /* RASTERIZER_H */

