#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H



typedef struct
{
    unsigned char* color;
    float* depth;
    int width;
    int height;
}
framebuffer;



/**
 * \brief Create a frame buffer object (32 bpp RGBA + 32 bit depth buffer)
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
 * \param fb A pointer to a frame buffer structure
 */
void framebuffer_destroy( framebuffer* fb );

/**
 * \brief Clear the color buffer of a frame buffer object
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
 * \param fb    A pointer to a frame buffer structure
 * \param value The value to write into the depth buffer
 */
void framebuffer_clear_depth( framebuffer* fb, float value );



#endif /* FRAMEBUFFER_H */

