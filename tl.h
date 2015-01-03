#ifndef TL_H
#define TL_H



#include "rasterizer.h"



#define MAX_LIGHTS 8



typedef struct
{
    float ambient[ 3 ];
    float diffuse[ 3 ];
    float specular[ 3 ];
    float position[ 3 ];
    float attenuation_constant;
    float attenuation_linear;
    float attenuation_quadratic;

    int enable;
}
tl_light;

typedef struct
{
    float ambient[ 3 ];
    float diffuse[ 3 ];
    float specular[ 3 ];
    float emission[ 3 ];
    int shininess;
}
tl_material;

typedef struct
{
    tl_light light[ MAX_LIGHTS ];
    tl_material material;
    int light_enable;
}
tl_state;



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
 * \param t A pointer to a triangle structure
 */
void tl_transform_and_light_triangle( triangle* t );



#endif /* TL_H */

