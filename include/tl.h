/**
 * \file pixel.h
 *
 * \brief Contains the fixed function transform & light stage interface
 */
#ifndef TL_H
#define TL_H



#include "rasterizer.h"



#define MAX_LIGHTS 8



/**
 * \struct tl_light
 *
 * \brief The settings of a light
 */
typedef struct
{
    float ambient[ 3 ];         /**< \brief Constant ambient term */
    float diffuse[ 3 ];         /**< \brief Diffuse color */
    float specular[ 3 ];        /**< \brief Specular color */
    float position[ 3 ];        /**< \brief View space light position */
    float attenuation_constant; /**< \brief Constant attenuation factor */
    float attenuation_linear;   /**< \brief Linear attenuation coefficient */
    float attenuation_quadratic;/**< \brief Quadratic attenuation coeff. */

    int enable;                 /**< \brief Non-zero if enabled */
}
tl_light;

/**
 * \struct tl_material
 *
 * \brief The surface material parameters
 */
typedef struct
{
    float ambient[ 3 ];     /**< \brief Ambient reflection color */
    float diffuse[ 3 ];     /**< \brief Diffuse reflection color */
    float specular[ 3 ];    /**< \brief Specular reflection color */
    float emission[ 3 ];    /**< \brief Color of light emitted */
    int shininess;          /**< \brief Specular exponent */
}
tl_material;

/**
 * \struct tl_state
 *
 * \brief The transform & lighting stage state
 */
typedef struct
{
    tl_light light[ MAX_LIGHTS ];  /**< \brief Data of all lights */
    tl_material material;          /**< \brief Data of the surface material */
    int light_enable;              /**< \brief Non-zero if lighting enabled */
}
tl_state;



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Set the currently active model view matrix
 *
 * \param f A pointer to an array of 16 float values, representing a 4x4
 *          matrix, stored in column-major order
 */
void tl_set_modelview_matrix( float* f );

/**
 * \brief Set the currently active projection matrix
 *
 * \param f A pointer to an array of 16 float values, representing a 4x4
 *          matrix, stored in column-major order
 */
void tl_set_projection_matrix( float* f );

/**
 * \brief Set the currently active transform & light state
 *
 * \param s A pointer to a T&L state structure
 */
void tl_set_state( const tl_state* s );

/**
 * \brief Transform the vertices of a triangle and compute lighting values
 *
 * \param v0 A pointer to the first vertex of a triangle
 * \param v1 A pointer to the second vertex of a triangle
 * \param v2 A pointer to the third vertex of a triangle
 */
void tl_transform_and_light_triangle( rs_vertex* v0, rs_vertex* v1,
                                      rs_vertex* v2 );

#ifdef __cplusplus
}
#endif

#endif /* TL_H */

