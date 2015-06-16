/**
 * \file texture.h
 *
 * \brief Contains the texture object implementation
 */
#ifndef TEXTURE_H
#define TEXTURE_H



#include "predef.h"



/**
 * \struct texture
 *
 * \brief Holds the data of a texture
 */
struct texture
{
    unsigned int width;     /**< \brief The width of a texture in texels */
    unsigned int height;    /**< \brief The height of  a texture in texels */
    unsigned char* data;    /**< \brief The scan lines of a texture */
};



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Create a texture object
 *
 * \memberof texture
 *
 * \param width  The width of the texture in pixels
 * \param height The height of the texture in pixels
 *
 * \return A pointer to a texture structure on success, NULL on error
 */
texture* texture_create( unsigned int width, unsigned int height );

/**
 * \brief Destroy a texture object
 *
 * \memberof texture
 *
 * \param t A pointer to a texture structure
 */
void texture_destroy( texture* t );

/**
 * \brief Read a sample from a texture object
 *
 * \memberof texture
 *
 * \param t   A pointer to a texture structure
 * \param tc  Texture coordinate in the range [0,1], where (0,0) is top left.
 * \param out A pointer to write the resulting color value to.
 */
void texture_sample( texture* t, const vec4* tc, vec4* out );

#ifdef __cplusplus
}
#endif

#endif /* TEXTURE_H */

