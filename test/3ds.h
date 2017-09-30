#ifndef LOADER_3DS_H
#define LOADER_3DS_H


typedef struct {
	float *vertexbuffer;
	unsigned short *indexbuffer;
	int format;
	unsigned int vertices;
	unsigned int indices;
} mesh;


#ifdef __cplusplus
extern "C" {
#endif

mesh *load_3ds(const char *filename);

#ifdef __cplusplus
}
#endif

#endif /* LOADER_3DS_H */

