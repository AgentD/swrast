#ifndef TEXTURE_H
#define TEXTURE_H



typedef struct
{
    unsigned int width, height;
    unsigned char* data;
}
texture;



/**
 * \brief Create a texture object
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
 * \param t A pointer to a texture structure
 */
void texture_destroy( texture* t );

/**
 * \brief Read a sample from a texture object
 *
 * \param t   A pointer to a texture structure
 * \param x   A value in the range [0,1]. 0=left, 1=right
 * \param y   A value in the range [0,1]. 0=top, 1=bottom
 * \param out A pointer to write the resulting color value to
 */
void texture_sample( texture* t, float x, float y, unsigned char* out );



#endif /* TEXTURE_H */

