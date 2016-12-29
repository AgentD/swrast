#include "framebuffer.h"
#include "rasterizer.h"
#include "context.h"
#include "texture.h"
#include "shader.h"
#include <math.h>

typedef struct {
	int left;                       /* index of left edge */
	int right;                      /* index of right edge */

	struct {
		vec4 v;                     /* current vertex */
		vec4 dvdy;                  /* difference per line */
	} edge[2];
} edge_data;

static void draw_scanline(int y, const context *ctx, const edge_data *s,
			vec4 color)
{
	float sub_pixel, *z_buffer, pixelscale, z, dzdx;
	unsigned char *start, *end;
	int x0, x1;

	/* get line start and end */
	x0 = ceil(s->edge[s->left].v.x);
	x1 = ceil(s->edge[s->right].v.x);

	sub_pixel = (float)x0 - s->edge[s->left].v.x;

	/* compuate start values and slopes */
	pixelscale = 1.0f / (s->edge[s->right].v.x - s->edge[s->left].v.x);

	dzdx = (s->edge[s->right].v.z - s->edge[s->left].v.z) * pixelscale;
	z = s->edge[s->left].v.z + dzdx * sub_pixel;

	/* get pointers to target scan line */
	if (x0 < ctx->draw_area.minx) {
		z += dzdx * (ctx->draw_area.minx - x0);
		x0 = ctx->draw_area.minx;
	}

	if (x1 > ctx->draw_area.maxx)
		x1 = ctx->draw_area.maxx;

	if (x1 < x0 || x0 > ctx->draw_area.maxx || x1 < ctx->draw_area.minx)
		return;

	z_buffer = ctx->target->depth + (y * ctx->target->width + x0);
	start = ctx->target->color + (y * ctx->target->width + x0) * 4;
	end = ctx->target->color + (y * ctx->target->width + x1) * 4;

	/* for each fragment */
	while (start != end && x0 <= ctx->draw_area.maxx) {
		if (depth_test(ctx, z, *z_buffer))
			write_fragment(ctx, color, z, start, z_buffer);
		z += dzdx;
		++z_buffer;
		start += 4;
		++x0;
	}
}

static void draw_half_triangle(edge_data *s, const vec4 A, const vec4 B,
				const vec4 color, context *ctx)
{
	float scale;
	int y0, y1;

	/* apply top-left fill convention */
	y0 = ceil(A.y);
	y1 = ceil(B.y) - 1;

	scale = (float)y0 - A.y;
	s->edge[0].v = vec4_add(s->edge[0].v,
				vec4_scale(s->edge[0].dvdy, scale));
	s->edge[1].v = vec4_add(s->edge[1].v,
				vec4_scale(s->edge[1].dvdy, scale));

	/* draw scanlines */
	if (y0 < ctx->draw_area.miny) {
		scale = ctx->draw_area.miny - y0;
		s->edge[0].v = vec4_add(s->edge[0].v,
					vec4_scale(s->edge[0].dvdy, scale));
		s->edge[1].v = vec4_add(s->edge[1].v,
					vec4_scale(s->edge[1].dvdy, scale));
		y0 = ctx->draw_area.miny;
	}

	for (; y0 <= y1 && y0<=ctx->draw_area.maxy; ++y0) {
		draw_scanline(y0, ctx, s, color);
		s->edge[0].v = vec4_add(s->edge[0].v, s->edge[0].dvdy);
		s->edge[1].v = vec4_add(s->edge[1].v, s->edge[1].dvdy);
	}
}

void draw_triangle_flat(vec4 A, vec4 B, vec4 C, vec4 color, context *ctx)
{
	float temp[4], linescale[3];
	vec4 temp_v;
	edge_data s;

	/* sort on Y axis */
	if (A.y > B.y) {
		temp_v = A;
		A = B;
		B = temp_v;
	}
	if (B.y > C.y) {
		temp_v = B;
		B = C;
		C = temp_v;
	}
	if (A.y > B.y) {
		temp_v = A;
		A = B;
		B = temp_v;
	}

	/* calculate y step per line */
	linescale[0] = 1.0f / (C.y - A.y);
	linescale[1] = 1.0f / (B.y - A.y);
	linescale[2] = 1.0f / (C.y - B.y);

	if (linescale[0] <= 0.0f)
		return;

	/* check if the major edge is left or right */
	temp[0] = A.x - C.x;
	temp[1] = A.y - C.y;
	temp[2] = B.x - A.x;
	temp[3] = B.y - A.y;

	s.left = (temp[0] * temp[3] - temp[1] * temp[2]) > 0.0f ? 0 : 1;
	s.right = !s.left;

	/* calculate slopes for major edge */
	s.edge[0].v = A;
	s.edge[0].dvdy = vec4_scale(vec4_sub(C, A), linescale[0]);

	/* rasterize upper sub-triangle */
	if (linescale[1] > 0.0f) {
		/* calculate slopes for minor edge */
		s.edge[1].v = A;
		s.edge[1].dvdy = vec4_scale(vec4_sub(B, A), linescale[1]);

		/* rasterize the edge scanlines */
		draw_half_triangle(&s, A, B, color, ctx);
	}

	/* rasterize lower sub-triangle */
	if (linescale[2] > 0.0f) {
		/* advance to center */
		if (linescale[1] > 0.0f) {
			temp[0] = B.y - A.y;
			s.edge[0].v = vec4_add(A,
					vec4_scale(s.edge[0].dvdy, temp[0]));
		}

		/* calculate slopes for bottom edge */
		s.edge[1].v = B;
		s.edge[1].dvdy = vec4_scale(vec4_sub(C, B), linescale[2]);

		draw_half_triangle(&s, B, C, color, ctx);
	}
}
