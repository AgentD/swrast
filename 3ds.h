#ifndef LOADER_3DS_H
#define LOADER_3DS_H



typedef struct
{
    float* vertexbuffer;            /**< \brief Pointer to the vertex data */
    unsigned short* indexbuffer;    /**< \brief Pointer to the index data */
    int format;                     /**< \brief Vertex format */
    unsigned int vertices;          /**< \brief Number of vertices */
    unsigned int indices;           /**< \brief Number of indices */
}
mesh;


mesh* load_3ds( const char* filename );



#endif /* LOADER_3DS_H */

