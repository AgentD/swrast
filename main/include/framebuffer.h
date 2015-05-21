/**
 * \file framebuffer.h
 *
 * \brief Contains a frame buffer object implementation
 */
#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H



#include "predef.h"
#include "config.h"



/**
 * \struct framebuffer
 *
 * \brief Holds the data of a frame buffer
 */
struct framebuffer
{
    unsigned char* color;   /**< \brief Color buffer scan line data */
    float* depth;           /**< \brief Depth buffer scan line data */
    int width;              /**< \brief Frame buffer width in pixels */
    int height;             /**< \brief Frame buffer height in pixels */
};



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Initialize an uninitialized frame buffer object
 *        (32 bpp RGBA + 32 bit depth buffer)
 *
 * \memberof framebuffer
 *
 * \param fb     A pointer to an uninitialized frame buffer object
 * \param width  The width of the frame buffer in pixels
 * \param height The height of the frame buffer in pixels
 *
 * \return Non-zero on success, zero on failure
 */
int framebuffer_init( framebuffer* fb,
                      unsigned int width, unsigned int height );

/**
 * \brief Destroy a frame buffer object and free its resources
 *
 * \memberof framebuffer
 *
 * \param fb A pointer to a frame buffer structure
 */
void framebuffer_cleanup( framebuffer* fb );

/**
 * \brief Clear the color buffer of a frame buffer object
 *
 * \memberof framebuffer
 *
 * \param fb A pointer to a frame buffer structure
 * \param r  The red component of the clear color
 * \param g  The green component of the clear color
 * \param b  The blue component of the clear color
 * \param a  The alpha component of the clear color
 */
void framebuffer_clear( framebuffer* fb, unsigned char r, unsigned char g,
                                         unsigned char b, unsigned char a );

/**
 * \brief Clear the depth buffer of a frame buffer object
 *
 * \memberof framebuffer
 *
 * \param fb    A pointer to a frame buffer structure
 * \param value The value to write into the depth buffer
 */
void framebuffer_clear_depth( framebuffer* fb, float value );

#ifdef __cplusplus
}
#endif

#endif /* FRAMEBUFFER_H */

