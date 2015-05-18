#ifndef CONTEXT_H
#define CONTEXT_H



#include "predef.h"
#include "config.h"



/**
 * \enum COMPARE_FUNCTION
 *
 * \brief Comparison function for depth test
 */
typedef enum
{
    COMPARE_ALWAYS = 0x00,          /**< \brief Always pass */
    COMPARE_NEVER = 0x01,           /**< \brief Always fail */
    COMPARE_EQUAL = 0x02,           /**< \brief Pass if equal */
    COMPARE_NOT_EQUAL = 0x03,       /**< \brief Pass if not equal */
    COMPARE_LESS = 0x04,            /**< \brief Pass if less */
    COMPARE_LESS_EQUAL = 0x05,      /**< \brief Pass if less or equal */
    COMPARE_GREATER = 0x06,         /**< \brief Pass if greater */
    COMPARE_GREATER_EQUAL = 0x07    /**< \brief Pass if greater or equal */
}
COMPARE_FUNCTION;

/**
 * \enum VERTEX_FORMAT
 *
 * \brief Vertex format flags
 */
typedef enum
{
    VF_POSITION_F2 = 0x0001,    /**< \brief 2 component float position */
    VF_POSITION_F3 = 0x0002,    /**< \brief 3 component float position */
    VF_POSITION_F4 = 0x0004,    /**< \brief 4 component float position */

    VF_NORMAL_F3 = 0x0010,      /**< \brief 3 component float normal */

    VF_COLOR_F3 = 0x0100,       /**< \brief 3 component float color */
    VF_COLOR_F4 = 0x0200,       /**< \brief 4 component float color */
    VF_COLOR_UB3 = 0x0400,      /**< \brief 3 component unsigned byte color */
    VF_COLOR_UB4 = 0x0800,      /**< \brief 4 component unsigned byte color */

    /**
     * \brief 2 component float texture coordinates for texture channel 0
     */
    VF_TEX0 = 0x1000
}
VERTEX_FORMAT;



struct context
{
    /** \brief The settings of a light */
    struct
    {
        float ambient[ 4 ];         /**< \brief Constant ambient term */
        float diffuse[ 4 ];         /**< \brief Diffuse color */
        float specular[ 4 ];        /**< \brief Specular color */
        float position[ 4 ];        /**< \brief View space light position */
        float attenuation_constant; /**< \brief Constant attenuation factor */
        float attenuation_linear;   /**< \brief Linear attenuation coeff. */
        float attenuation_quadratic;/**< \brief Quadratic attenuation coeff.*/
        int enable;                 /**< \brief Non-zero if enabled */
    }
    light[ MAX_LIGHTS ];

    /** \brief The surface material parameters for lighting calculations */
    struct
    {
        float ambient[ 4 ];     /**< \brief Ambient reflection color */
        float diffuse[ 4 ];     /**< \brief Diffuse reflection color */
        float specular[ 4 ];    /**< \brief Specular reflection color */
        float emission[ 4 ];    /**< \brief Color of light emitted */
        int shininess;          /**< \brief Specular exponent */
    }
    material;

    int light_enable;           /**< \brief Non-zero if lighting enabled */

    /** \brief Non-zero to cull counter clockwise triangles */
    int cull_ccw;

    /** \brief Non-zero to cull clockwise triangles */
    int cull_cw;

    /** \brief Depth test comparison function */
    COMPARE_FUNCTION depth_test;

    /** \brief Non-zero to enable alpha blending, zero to disable */
    int alpha_blend;

    /** \brief Non-zero to enable a texture layer, zero to disable */
    int texture_enable[MAX_TEXTURES];

    /** \brief Pointer to textures for different texture layers */
    texture* textures[MAX_TEXTURES];

    /** \brief Model-View matrix used by T&L stage */
    float modelview[16];

    /** \brief Projection matrix used by T&L stage */
    float projection[16];

    /** \brief Normal matrix computed from modelview matrix */
    float normalmatrix[16];

    /** \brief Input assembler vertex format */
    int vertex_format;

    /** \brief Vertex buffer for input assember */
    void* vertexbuffer;

    /** \brief Index buffer for input assembler */
    unsigned short* indexbuffer;

    /** \brief Frame buffer that the rasterizer draws to */
    framebuffer* target;
};



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Initialize a context object
 *
 * \memberof context
 *
 * \param ctx A pointer to a context
 */
void context_init( context* ctx );

/**
 * \brief Set the currently active model view matrix and update the
 *        normal matrix
 *
 * \memberof context
 *
 * \param ctx A pointer to a context
 * \param f   A pointer to an array of 16 float values, representing a 4x4
 *            matrix, stored in column-major order
 */
void context_set_modelview_matrix( context* ctx, float* f );

/**
 * \brief Set the currently active projection matrix
 *
 * \memberof context
 *
 * \param ctx A pointer to a context
 * \param f   A pointer to an array of 16 float values, representing a 4x4
 *            matrix, stored in column-major order
 */
void context_set_projection_matrix( context* ctx, float* f );

#ifdef __cplusplus
}
#endif

#endif /* CONTEXT_H */

