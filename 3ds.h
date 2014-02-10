#ifndef LOADER_3DS_H
#define LOADER_3DS_H



int load_3ds( const char* filename,
              void** vertexbuffer,
              unsigned short** indexbuffer,
              int* format,
              unsigned int* vertices,
              unsigned int* indices );



#endif /* LOADER_3DS_H */

