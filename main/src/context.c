#include "framebuffer.h"
#include "context.h"
#include <stddef.h>
#include <string.h>
#include <float.h>

static void recompute_normal_matrix(context *ctx)
{
	float det, m[16], f[18], *mv, *normal;
	int i, j;

	mv = ctx->modelview;
	normal = ctx->normalmatrix;

	/* m = inverse( mv ) */
	f[ 0] = mv[10] * mv[15] - mv[11] * mv[14];
	f[ 1] = mv[ 7] * mv[14] - mv[ 6] * mv[15];
	f[ 2] = mv[ 6] * mv[11] - mv[ 7] * mv[10];
	f[ 3] = mv[ 2] * mv[15] - mv[ 3] * mv[14];
	f[ 4] = mv[ 3] * mv[10] - mv[ 2] * mv[11];
	f[ 5] = mv[ 2] * mv[ 7] - mv[ 3] * mv[ 6];
	f[ 6] = mv[ 9] * mv[15] - mv[11] * mv[13];
	f[ 7] = mv[ 7] * mv[13] - mv[ 5] * mv[15];
	f[ 8] = mv[ 5] * mv[11] - mv[ 7] * mv[ 9];
	f[ 9] = mv[ 1] * mv[15] - mv[ 3] * mv[13];
	f[10] = mv[ 3] * mv[ 9] - mv[ 1] * mv[11];
	f[11] = mv[ 1] * mv[ 7] - mv[ 3] * mv[ 5];
	f[12] = mv[10] * mv[13] - mv[ 9] * mv[14];
	f[13] = mv[ 5] * mv[14] - mv[ 6] * mv[13];
	f[14] = mv[ 6] * mv[ 9] - mv[ 5] * mv[10];
	f[15] = mv[ 2] * mv[13] - mv[ 1] * mv[14];
	f[16] = mv[ 1] * mv[10] - mv[ 2] * mv[ 9];
	f[17] = mv[ 2] * mv[ 5] - mv[ 1] * mv[ 6];

	m[ 0] =  mv[5] * f[ 0] + mv[9] * f[ 1] + mv[13] * f[ 2];
	m[ 1] = -mv[1] * f[ 0] + mv[9] * f[ 3] + mv[13] * f[ 4];
	m[ 2] = -mv[1] * f[ 1] - mv[5] * f[ 3] + mv[13] * f[ 5];
	m[ 3] = -mv[1] * f[ 2] - mv[5] * f[ 4] - mv[ 9] * f[ 5];

	m[ 4] = -mv[4] * f[ 0] - mv[8] * f[ 1] - mv[12] * f[ 2];
	m[ 5] =  mv[0] * f[ 0] - mv[8] * f[ 3] - mv[12] * f[ 4];
	m[ 6] =  mv[0] * f[ 1] + mv[4] * f[ 3] - mv[12] * f[ 5];
	m[ 7] =  mv[0] * f[ 2] + mv[4] * f[ 4] + mv[ 8] * f[ 5];

	m[ 8] =  mv[4] * f[ 6] + mv[8] * f[ 7] + mv[12] * f[ 8];
	m[ 9] = -mv[0] * f[ 6] + mv[8] * f[ 9] + mv[12] * f[10];
	m[10] = -mv[0] * f[ 7] - mv[4] * f[ 9] + mv[12] * f[11];
	m[11] = -mv[0] * f[ 8] - mv[4] * f[10] - mv[ 8] * f[11];

	m[12] =  mv[4] * f[12] + mv[8] * f[13] + mv[12] * f[14];
	m[13] =  mv[0] * f[12] + mv[8] * f[15] + mv[12] * f[16];
	m[14] = -mv[0] * f[13] - mv[4] * f[15] + mv[12] * f[17];
	m[15] = -mv[0] * f[14] - mv[4] * f[16] - mv[ 8] * f[17];

	det = mv[0] * m[0] + mv[1] * m[4] + mv[2] * m[8] + mv[3] * m[12];

	/* normal = transpose(inverse(mv)) = transpose(m/det) */
	if ((det < -FLT_MIN) || (det > FLT_MIN)) {
		det = 1.0f / det;

		for (i = 0; i < 4; ++i) {
			for (j = 0; j < 4; ++j) {
				normal[i*4 + j] = m[j*4 + i] * det;
			}
		}
	} else {
		memset(normal, 0, sizeof(float) * 16);
		normal[0] = normal[5] = normal[10] = normal[15] = 1.0f;
	}
}

void context_init(context *ctx)
{
	int i;

	memset(ctx, 0, sizeof(*ctx));

	for (i = 0; i < MAX_LIGHTS; ++i) {
		ctx->light[i].position = vec4_set(0.0f, 0.0f, 0.0f, 1.0f);
		ctx->light[i].ambient = vec4_set(0.0f, 0.0f, 0.0f, 1.0f);
		ctx->light[i].diffuse = vec4_set(0.0f, 0.0f, 0.0f, 1.0f);
		ctx->light[i].specular = vec4_set(0.0f, 0.0f, 0.0f, 1.0f);
	}

	ctx->light[0].diffuse = vec4_set(1.0f, 1.0f, 1.0f, 1.0f);
	ctx->light[0].specular = vec4_set(1.0f, 1.0f, 1.0f, 1.0f);

	ctx->material.ambient = vec4_set(1.0f, 1.0f, 1.0f, 1.0f);
	ctx->material.diffuse = vec4_set(1.0f, 1.0f, 1.0f, 1.0f);
	ctx->material.specular = vec4_set(1.0f, 1.0f, 1.0f, 1.0f);
	ctx->material.emission = vec4_set(0.0f, 0.0f, 0.0f, 1.0f);

	ctx->projection[ 0] = ctx->modelview[ 0] = ctx->normalmatrix[ 0] = 1.0f;
	ctx->projection[ 5] = ctx->modelview[ 5] = ctx->normalmatrix[ 5] = 1.0f;
	ctx->projection[10] = ctx->modelview[10] = ctx->normalmatrix[10] = 1.0f;
	ctx->projection[15] = ctx->modelview[15] = ctx->normalmatrix[15] = 1.0f;

	ctx->shader = SHADER_PHONG;
	ctx->shade_mode = SHADE_PER_VERTEX;
	ctx->depth_test = COMPARE_ALWAYS;
	ctx->depth_far = 1.0f;
	ctx->flags = DEPTH_CLIP|DEPTH_WRITE|WRITE_COLOR|FRONT_CCW;
}

void context_set_modelview_matrix(context *ctx, float *f)
{
	memcpy(ctx->modelview, f, sizeof(float) * 16);
	recompute_normal_matrix(ctx);
}

void context_set_projection_matrix(context *ctx, float *f)
{
	memcpy(ctx->projection, f, sizeof(float) * 16);
}

void context_set_viewport(context *ctx, int x, int y,
			unsigned int width, unsigned int height)
{
	ctx->viewport.x = ctx->draw_area.minx = x;
	ctx->viewport.y = ctx->draw_area.miny = y;
	ctx->viewport.width = width;
	ctx->viewport.height = height;

	ctx->draw_area.maxx = x + width - 1;
	ctx->draw_area.maxy = y + height - 1;

	if (ctx->draw_area.maxx < 0)
		ctx->draw_area.maxx = 0;
	if (ctx->draw_area.maxy < 0)
		ctx->draw_area.maxy = 0;
	if (ctx->draw_area.minx < 0)
		ctx->draw_area.minx = 0;
	if (ctx->draw_area.miny < 0)
		ctx->draw_area.miny = 0;

	if (ctx->draw_area.maxx >= (int)ctx->target->width)
		ctx->draw_area.maxx = ctx->target->width - 1;
	if (ctx->draw_area.maxy >= (int)ctx->target->height)
		ctx->draw_area.maxy = ctx->target->height - 1;
	if (ctx->draw_area.minx >= (int)ctx->target->width)
		ctx->draw_area.minx = ctx->target->width - 1;
	if (ctx->draw_area.miny >= (int)ctx->target->height)
		ctx->draw_area.miny = ctx->target->height - 1;
}
