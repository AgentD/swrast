#include "inputassembler.h"
#include "3ds.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>



#define MAIN_CHUNK           0x4D4D /* the file itself(magic value) */
#define EDITOR_CHUNK         0x3D3D /* the geometry data, edit config, ... */
#define KEYFRAME_CHUNK       0xB000 /* animation data */


#define OBJECT_CHUNK         0x4000 /* the scene description */

#define MESH_CHUNK           0x4100 /* a mesh in the scene */

#define VERTEX_LIST          0x4110 /* vertex data of a mesh */
#define FACE_DESCRIPTION     0x4120 /* index data of a mesh */
#define MAPPING_COORD_LIST   0x4140 /* texture coordinates of a mesh */



#define LIL_TO_HOST_16( v )
#define LIL_TO_HOST_32( v )
#define LIL_TO_HOST_32F( v )



int load_3ds( const char* filename,
              void** vertexbuffer,
              unsigned short** indexbuffer,
              int* format,
              unsigned int* vertices,
              unsigned int* indices )
{
    uint16_t chunk_name;
    uint32_t chunk_size;
    uint32_t chunk_start;
    float* vertex_data = 0;
    float* texture_data = 0;
    unsigned int i;
    FILE* fp;

    /* parameter sanity checks */
    if( !filename || !vertexbuffer || !indexbuffer ||
        !format   || !vertices     || !indices )
    {
        return 0;
    }

    *vertexbuffer = 0;
    *indexbuffer = 0;
    *vertices = 0;
    *indices = 0;
    *format = VF_POSITION_F3;

    if( !(fp = fopen( filename, "rb" )) )
        return 0;

    /* check the file's magic value */
    fread( &chunk_name, 2, 1, fp );

    LIL_TO_HOST_16( chunk_name );

    if( chunk_name != MAIN_CHUNK )
    {
        fclose( fp );
        return 0;
    }

    fread( &chunk_size, 4, 1, fp );

    /* read and parse the chunks */
    for( ; ; )
    {
        /* read the chunk header data */
        chunk_start = ftell( fp );

        fread( &chunk_name, 2, 1, fp );
        LIL_TO_HOST_16( chunk_name );

        fread( &chunk_size, 4, 1, fp );
        LIL_TO_HOST_16( chunk_size );

        if( feof( fp ) )
            break;

        /* parse the chunk */
        switch( chunk_name )
        {
        case EDITOR_CHUNK:
        case MESH_CHUNK:
            continue;
        case OBJECT_CHUNK:
        {
            char c;
            do { fread( &c, 1, 1, fp ); } while( c );
            continue;
        }
        case VERTEX_LIST:
        {
            uint16_t num_vertices;

            if( vertex_data )
            {
                fseek( fp, chunk_start + chunk_size, SEEK_SET );
                continue;
            }

            fread( &num_vertices, 2, 1, fp );
            LIL_TO_HOST_16( num_vertices );

            vertex_data = malloc( num_vertices * 3 * sizeof(float) );
            fread( vertex_data, 3*sizeof(float), num_vertices, fp );

            *vertices = num_vertices;
            continue;
        }
        case FACE_DESCRIPTION:
        {
            uint16_t num_indices;
            uint16_t flags;

            if( *indexbuffer )
            {
                fseek( fp, chunk_start + chunk_size, SEEK_SET );
                continue;
            }

            fread( &num_indices, 2, 1, fp );
            LIL_TO_HOST_16( num_indices );

            *indexbuffer = malloc( num_indices * 2 * 3 );
            *indices = num_indices * 3;

            for( i=0; i<num_indices; ++i )
            {
                fread( &((*indexbuffer)[ 3*i ]), 2, 3, fp );
                fread( &flags, 2, 1, fp );

                LIL_TO_HOST_16( (*indexbuffer)[ 3*i     ] );
                LIL_TO_HOST_16( (*indexbuffer)[ 3*i + 1 ] );
                LIL_TO_HOST_16( (*indexbuffer)[ 3*i + 2 ] );
            }
            continue;
        }
        case MAPPING_COORD_LIST:
        {
            uint16_t nvert;

            if( texture_data )
            {
                fseek( fp, chunk_start + chunk_size, SEEK_SET );
                continue;
            }

            fread( &nvert, 2, 1, fp );
            LIL_TO_HOST_16( nvert );

            texture_data = malloc( nvert * 2 * sizeof(float) );
            fread( texture_data, 2*sizeof(float), nvert, fp );
            continue;
        }
        default:
            fseek( fp, chunk_start + chunk_size, SEEK_SET );
        };
    }

    fclose( fp );

    /* final sanity check if we have everything we expect */
    if( !vertex_data || !(*indexbuffer) )
    {
        free( vertex_data  );
        free( texture_data );
        free( *indexbuffer );
        return 0;
    }

    /* determine the missing data and combine the buffers */
    if( texture_data )
    {
        *format = VF_POSITION_F3 | VF_NORMAL_F3 | VF_TEX0;
        *vertexbuffer = malloc( (*vertices) * sizeof(float) * 8 );

        /* fill the final vertex buffer */
        for( i=0; i<(*vertices); ++i )
        {
            LIL_TO_HOST_32F( texture_data[ 2*i     ] );
            LIL_TO_HOST_32F( texture_data[ 2*i + 1 ] );
            LIL_TO_HOST_32F( vertex_data[ 3*i     ] );
            LIL_TO_HOST_32F( vertex_data[ 3*i + 1 ] );
            LIL_TO_HOST_32F( vertex_data[ 3*i + 2 ] );

            ((float*)*vertexbuffer)[8*i  ] = vertex_data[ 3*i     ];
            ((float*)*vertexbuffer)[8*i+1] = vertex_data[ 3*i + 1 ];
            ((float*)*vertexbuffer)[8*i+2] = vertex_data[ 3*i + 2 ];
            ((float*)*vertexbuffer)[8*i+3] = 0.0f;
            ((float*)*vertexbuffer)[8*i+4] = 0.0f;
            ((float*)*vertexbuffer)[8*i+5] = 0.0f;
            ((float*)*vertexbuffer)[8*i+6] = texture_data[ 2*i     ];
            ((float*)*vertexbuffer)[8*i+7] = texture_data[ 2*i + 1 ];
        }

        /* calculate surface normals */
        for( i=0; i<(*indices); i+=3 )
        {
            float v1[3], v2[3], nx, ny, nz;
            uint16_t a, b, c;

            a = (*indexbuffer)[i  ];
            b = (*indexbuffer)[i+1];
            c = (*indexbuffer)[i+2];

            v1[0] = vertex_data[ 3*a     ] - vertex_data[ 3*b     ];
            v1[1] = vertex_data[ 3*a + 1 ] - vertex_data[ 3*b + 1 ];
            v1[2] = vertex_data[ 3*a + 2 ] - vertex_data[ 3*b + 2 ];

            v2[0] = vertex_data[ 3*a     ] - vertex_data[ 3*c     ];
            v2[1] = vertex_data[ 3*a + 1 ] - vertex_data[ 3*c + 1 ];
            v2[2] = vertex_data[ 3*a + 2 ] - vertex_data[ 3*c + 2 ];

            nx = v1[1]*v2[2] - v1[2]*v2[1];
            ny = v1[2]*v2[0] - v1[0]*v2[2];
            nz = v1[0]*v2[1] - v1[1]*v2[0];

            ((float*)*vertexbuffer)[8*a+3] += nx;
            ((float*)*vertexbuffer)[8*a+4] += ny;
            ((float*)*vertexbuffer)[8*a+5] += nz;

            ((float*)*vertexbuffer)[8*b+3] += nx;
            ((float*)*vertexbuffer)[8*b+4] += ny;
            ((float*)*vertexbuffer)[8*b+5] += nz;

            ((float*)*vertexbuffer)[8*c+3] += nx;
            ((float*)*vertexbuffer)[8*c+4] += ny;
            ((float*)*vertexbuffer)[8*c+5] += nz;
        }

        /* normalize the normals */
        for( i=0; i<(*vertices); ++i )
        {
            float l;

            l = ((float*)*vertexbuffer)[i*8+3] *
                ((float*)*vertexbuffer)[i*8+3] +
                ((float*)*vertexbuffer)[i*8+4] *
                ((float*)*vertexbuffer)[i*8+4] +
                ((float*)*vertexbuffer)[i*8+5] *
                ((float*)*vertexbuffer)[i*8+5];

            l = 1.0f / sqrt( l );

            ((float*)*vertexbuffer)[i*8+3] *= l;
            ((float*)*vertexbuffer)[i*8+4] *= l;
            ((float*)*vertexbuffer)[i*8+5] *= l;
        }
    }
    else
    {
        *format = VF_POSITION_F3 | VF_NORMAL_F3;
        *vertexbuffer = malloc( (*vertices) * sizeof(float) * 6 );

        /* fill the final vertex buffer */
        for( i=0; i<(*vertices); ++i )
        {
            LIL_TO_HOST_32F( vertex_data[ 3*i     ] );
            LIL_TO_HOST_32F( vertex_data[ 3*i + 1 ] );
            LIL_TO_HOST_32F( vertex_data[ 3*i + 2 ] );

            ((float*)*vertexbuffer)[i*6  ] = vertex_data[ 3*i     ];
            ((float*)*vertexbuffer)[i*6+1] = vertex_data[ 3*i + 1 ];
            ((float*)*vertexbuffer)[i*6+2] = vertex_data[ 3*i + 2 ];
            ((float*)*vertexbuffer)[i*6+3] = 0.0f;
            ((float*)*vertexbuffer)[i*6+4] = 0.0f;
            ((float*)*vertexbuffer)[i*6+5] = 0.0f;
        }

        /* calculate surface normals */
        for( i=0; i<(*indices); i+=3 )
        {
            float v1[3], v2[3], nx, ny, nz;
            uint16_t a, b, c;

            a = (*indexbuffer)[i  ];
            b = (*indexbuffer)[i+1];
            c = (*indexbuffer)[i+2];

            v1[0] = vertex_data[ 3*a     ] - vertex_data[ 3*b     ];
            v1[1] = vertex_data[ 3*a + 1 ] - vertex_data[ 3*b + 1 ];
            v1[2] = vertex_data[ 3*a + 2 ] - vertex_data[ 3*b + 2 ];

            v2[0] = vertex_data[ 3*a     ] - vertex_data[ 3*c     ];
            v2[1] = vertex_data[ 3*a + 1 ] - vertex_data[ 3*c + 1 ];
            v2[2] = vertex_data[ 3*a + 2 ] - vertex_data[ 3*c + 2 ];

            nx = v1[1]*v2[2] - v1[2]*v2[1];
            ny = v1[2]*v2[0] - v1[0]*v2[2];
            nz = v1[0]*v2[1] - v1[1]*v2[0];

            ((float*)*vertexbuffer)[a*6+3] += nx;
            ((float*)*vertexbuffer)[a*6+4] += ny;
            ((float*)*vertexbuffer)[a*6+5] += nz;

            ((float*)*vertexbuffer)[b*6+3] += nx;
            ((float*)*vertexbuffer)[b*6+4] += ny;
            ((float*)*vertexbuffer)[b*6+5] += nz;

            ((float*)*vertexbuffer)[c*6+3] += nx;
            ((float*)*vertexbuffer)[c*6+4] += ny;
            ((float*)*vertexbuffer)[c*6+5] += nz;
        }

        /* normalize the normals */
        for( i=0; i<(*vertices); ++i )
        {
            float l;

            l = ((float*)*vertexbuffer)[i*6+3] *
                ((float*)*vertexbuffer)[i*6+3] +
                ((float*)*vertexbuffer)[i*6+4] *
                ((float*)*vertexbuffer)[i*6+4] +
                ((float*)*vertexbuffer)[i*6+5] *
                ((float*)*vertexbuffer)[i*6+5];

            l = 1.0f / sqrt( l );

            ((float*)*vertexbuffer)[i*6+3] *= l;
            ((float*)*vertexbuffer)[i*6+4] *= l;
            ((float*)*vertexbuffer)[i*6+5] *= l;
        }
    }

    /* cleanup */
    free( vertex_data  );
    free( texture_data );
    return 1;
}


