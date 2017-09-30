#include "inputassembler.h"
#include "context.h"
#include "3ds.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


#define MAIN_CHUNK         0x4D4D
#define EDITOR_CHUNK       0x3D3D
#define KEYFRAME_CHUNK     0xB000
#define OBJECT_CHUNK       0x4000
#define MESH_CHUNK         0x4100
#define VERTEX_LIST        0x4110
#define FACE_DESCRIPTION   0x4120
#define MAPPING_COORD_LIST 0x4140


#define LIL_TO_HOST_16(v)
#define LIL_TO_HOST_32(v)
#define LIL_TO_HOST_32F(v)


static void gen_average_normals(mesh *this, int vs)
{
	float v1[3], v2[3], nx, ny, nz;
	unsigned int i, a, b, c;

	for (i = 0; i < this->indices; i += 3) {
		a = this->indexbuffer[i  ];
		b = this->indexbuffer[i+1];
		c = this->indexbuffer[i+2];

		v1[0] = this->vertexbuffer[vs*a  ]-this->vertexbuffer[vs*b  ];
		v1[1] = this->vertexbuffer[vs*a+1]-this->vertexbuffer[vs*b+1];
		v1[2] = this->vertexbuffer[vs*a+2]-this->vertexbuffer[vs*b+2];

		v2[0] = this->vertexbuffer[vs*a  ]-this->vertexbuffer[vs*c  ];
		v2[1] = this->vertexbuffer[vs*a+1]-this->vertexbuffer[vs*c+1];
		v2[2] = this->vertexbuffer[vs*a+2]-this->vertexbuffer[vs*c+2];

		nx = v1[1]*v2[2] - v1[2]*v2[1];
		ny = v1[2]*v2[0] - v1[0]*v2[2];
		nz = v1[0]*v2[1] - v1[1]*v2[0];

		this->vertexbuffer[a*vs+3] += nx;
		this->vertexbuffer[a*vs+4] += ny;
		this->vertexbuffer[a*vs+5] += nz;

		this->vertexbuffer[b*vs+3] += nx;
		this->vertexbuffer[b*vs+4] += ny;
		this->vertexbuffer[b*vs+5] += nz;

		this->vertexbuffer[c*vs+3] += nx;
		this->vertexbuffer[c*vs+4] += ny;
		this->vertexbuffer[c*vs+5] += nz;
	}
}

static void normalize_normals(mesh *this, int vs)
{
	unsigned int i;
	float l;

	for (i = 0; i < this->vertices; ++i) {
		l  = this->vertexbuffer[i*vs+3] * this->vertexbuffer[i*vs+3];
		l += this->vertexbuffer[i*vs+4] * this->vertexbuffer[i*vs+4];
		l += this->vertexbuffer[i*vs+5] * this->vertexbuffer[i*vs+5];

		l = 1.0f / sqrt(l);

		this->vertexbuffer[i*vs+3] *= l;
		this->vertexbuffer[i*vs+4] *= l;
		this->vertexbuffer[i*vs+5] *= l;
	}
}

mesh *load_3ds(const char *filename)
{
	float *vertex_data = NULL, *texture_data = NULL;
	uint32_t chunk_size, chunk_start;
	uint16_t chunk_name, count, temp;
	unsigned int i, vs;
	mesh *this;
	FILE *fp;
	char c;

	if (!filename)
		return NULL;

	this = calloc(1, sizeof(*this));
	if (!this)
		return NULL;

	if (!(fp = fopen(filename, "rb"))) {
		free(this);
		return NULL;
	}

	/* check the file's magic value */
	fread(&chunk_name, 2, 1, fp);
	LIL_TO_HOST_16(chunk_name);

	if (chunk_name != MAIN_CHUNK) {
		free(this);
		fclose(fp);
		return NULL;
	}

	fread(&chunk_size, 4, 1, fp);

	/* read and parse the chunks */
	for (;;) {
		chunk_start = ftell(fp);

		fread(&chunk_name, 2, 1, fp);
		fread(&chunk_size, 4, 1, fp);

		LIL_TO_HOST_16(chunk_name);
		LIL_TO_HOST_16(chunk_size);

		if (feof(fp))
			break;

		switch (chunk_name) {
		case EDITOR_CHUNK:
		case MESH_CHUNK:
			continue;
		case OBJECT_CHUNK:
			do { fread( &c, 1, 1, fp ); } while( c );
			continue;
		case VERTEX_LIST:
			if (vertex_data) {
				fseek(fp, chunk_start + chunk_size, SEEK_SET);
				continue;
			}

			fread(&count, 2, 1, fp);
			LIL_TO_HOST_16(count);

			vertex_data = malloc(count * 3 * sizeof(float));
			fread(vertex_data, 3 * sizeof(float), count, fp);

			this->vertices = count;
			continue;
		case FACE_DESCRIPTION:
			if (this->indexbuffer) {
				fseek(fp, chunk_start + chunk_size, SEEK_SET);
				continue;
			}

			fread(&count, 2, 1, fp);
			LIL_TO_HOST_16(count);

			this->indexbuffer = malloc(count * 2 * 3);
			this->indices = count * 3;

			for (i = 0; i < count; ++i) {
				fread(&(this->indexbuffer[3 * i]), 2, 3, fp);
				fread(&temp, 2, 1, fp);

				LIL_TO_HOST_16(this->indexbuffer[3 * i    ]);
				LIL_TO_HOST_16(this->indexbuffer[3 * i + 1]);
				LIL_TO_HOST_16(this->indexbuffer[3 * i + 2]);
			}
			continue;
		case MAPPING_COORD_LIST:
			if (texture_data) {
				fseek(fp, chunk_start + chunk_size, SEEK_SET);
				continue;
			}

			fread(&count, 2, 1, fp);
			LIL_TO_HOST_16(count);

			texture_data = malloc(count * 2 * sizeof(float));
			fread(texture_data, 2 * sizeof(float), count, fp);
			continue;
		default:
			fseek(fp, chunk_start + chunk_size, SEEK_SET);
		}
	}

	fclose(fp);

	/* final sanity check if we have everything we expect */
	if (!vertex_data || !this->indexbuffer) {
		free(vertex_data);
		free(texture_data);
		free(this->indexbuffer);
		free(this);
		return NULL;
	}

	/* determine the missing data and combine the buffers */
	vs = texture_data ? 8 : 6;
	this->format = (texture_data ? VF_TEX0 : 0) |
			VF_POSITION_F3 | VF_NORMAL_F3;

	this->vertexbuffer = malloc(this->vertices * sizeof(float) * vs);

	for (i = 0; i < this->vertices; ++i) {
		LIL_TO_HOST_32F(vertex_data[3 * i    ]);
		LIL_TO_HOST_32F(vertex_data[3 * i + 1]);
		LIL_TO_HOST_32F(vertex_data[3 * i + 2]);

		this->vertexbuffer[vs * i    ] = vertex_data[3 * i    ];
		this->vertexbuffer[vs * i + 1] = vertex_data[3 * i + 1];
		this->vertexbuffer[vs * i + 2] = vertex_data[3 * i + 2];
		this->vertexbuffer[vs * i + 3] = 0.0f;
		this->vertexbuffer[vs * i + 4] = 0.0f;
		this->vertexbuffer[vs * i + 5] = 0.0f;
	}

	if (texture_data) {
		for (i = 0; i < this->vertices; ++i) {
			LIL_TO_HOST_32F(texture_data[2 * i    ]);
			LIL_TO_HOST_32F(texture_data[2 * i + 1]);

			this->vertexbuffer[8*i + 6] = texture_data[2*i    ];
			this->vertexbuffer[8*i + 7] = texture_data[2*i + 1];
		}
	}

	gen_average_normals(this, vs);
	normalize_normals(this, vs);

	free(vertex_data);
	free(texture_data);
	return this;
}
