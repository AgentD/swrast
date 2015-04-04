/**
 * \file pixel.h
 *
 * \brief Contains the rasterizer stage interface
 */
#ifndef RASTERIZER_H
#define RASTERIZER_H



#include "framebuffer.h"
#include "texture.h"



#define MAX_TEXTURES 1

#define COMPARE_ALWAYS 0x00
#define COMPARE_NEVER 0x01
#define COMPARE_EQUAL 0x02
#define COMPARE_NOT_EQUAL 0x03
#define COMPARE_LESS 0x04
#define COMPARE_LESS_EQUAL 0x05
#define COMPARE_GREATER 0x06
#define COMPARE_GREATER_EQUAL 0x07



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

/**
 * \struct pixel_state
 *
 * \brief Encapsulates the state of the pixel processor
 */
typedef struct
{
    /** \brief Depth test comparison function */
    int depth_test;

    /** \brief Non-zero to enable alpha blending, zero to disable */
    int alpha_blend;

    /** \brief Non-zero to enable a texture layer, zero to disable */
    int texture_enable[MAX_TEXTURES];

    /** \brief Pointer to textures for different texture layers */
    texture* textures[MAX_TEXTURES];
}
pixel_state;




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
 * \brief Overwrite the current pixel processor state
 *
 * \param s A pointer to a state structure
 */
void pixel_set_state( const pixel_state* s );

/**
 * \brief Read the current pixel processor state
 *
 * \param s A pointer to a state structure to write the current state to
 */
void pixel_get_state( pixel_state* s );

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

