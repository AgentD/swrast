/**
 * \file framebuffer.h
 *
 * \brief Contains a frame buffer object implementation
 */
#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H



#define FB_BGRA

#ifdef FB_BGRA
    #define RED 2
    #define GREEN 1
    #define BLUE 0
    #define ALPHA 3
#else
    #define RED 0
    #define GREEN 1
    #define BLUE 2
    #define ALPHA 3
#endif



#define DEPTH_MAX 0x00FFFFFF
#define DEPTH_MAX_HALF 0x007FFFFF



/**
 * \struct framebuffer
 *
 * \brief Holds the data of a frame buffer
 */
typedef struct
{
    unsigned char* color;   /**< \brief Color buffer scan line data */
    int* depth;             /**< \brief Depth buffer scan line data */
    int width;              /**< \brief Frame buffer width in pixels */
    int height;             /**< \brief Frame buffer height in pixels */
}
framebuffer;



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Create a frame buffer object (32 bpp RGBA + 32 bit depth buffer)
 *
 * \memberof framebuffer
 *
 * \param width  The width of the frame buffer in pixels
 * \param height The height of the frame buffer in pixels
 *
 * \return A pointer to a framebuffer structure on success, NULL on failure
 */
framebuffer* framebuffer_create( unsigned int width, unsigned int height );

/**
 * \brief Destroy a frame buffer object and free its resources
 *
 * \memberof framebuffer
 *
 * \param fb A pointer to a frame buffer structure
 */
void framebuffer_destroy( framebuffer* fb );

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

