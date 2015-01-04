/**
 * \file 3ds.h
 *
 * \brief Contains a 3ds loader implementation for testing
 */
#ifndef LOADER_3DS_H
#define LOADER_3DS_H



/**
 * \struct mesh
 *
 * \brief A polygon mesh object
 */
typedef struct
{
    float* vertexbuffer;            /**< \brief Pointer to the vertex data */
    unsigned short* indexbuffer;    /**< \brief Pointer to the index data */
    int format;                     /**< \brief Vertex format */
    unsigned int vertices;          /**< \brief Number of vertices */
    unsigned int indices;           /**< \brief Number of indices */
}
mesh;



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Load a mesh from a 3DS file
 *
 * \memberof mesh
 *
 * \param filename The path of the file to load
 *
 * \return A pointer to a mesh strcture on success, NULL on failure
 */
mesh* load_3ds( const char* filename );

#ifdef __cplusplus
}
#endif

#endif /* LOADER_3DS_H */

