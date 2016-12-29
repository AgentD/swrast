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
struct texture {
	unsigned int width;
	unsigned int height;
	unsigned char *data;
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
texture *texture_create(unsigned int width, unsigned int height);

/**
 * \brief Destroy a texture object
 *
 * \memberof texture
 *
 * \param t A pointer to a texture structure
 */
void texture_destroy(texture *t);

/**
 * \brief Read a sample from a texture object
 *
 * \memberof texture
 *
 * \param t   A pointer to a texture structure
 * \param tc  Texture coordinate in the range [0,1], where (0,0) is top left.
 *
 * \return The resulting color value.
 */
MATH_CONST vec4 texture_sample(const texture  *t, const vec4 tc);

#ifdef __cplusplus
}
#endif

#endif /* TEXTURE_H */

