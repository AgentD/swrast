#include "framebuffer.h"
#include "rasterizer.h"
#include "context.h"
#include "texture.h"
#include "shader.h"
#include "color.h"
#include <math.h>

extern void draw_triangle_per_pixel(rs_vertex *A, rs_vertex *B,
					rs_vertex *C, context *ctx);

void write_fragment(const context *ctx, const color4 frag_color,
		float frag_depth, color4 *color_buffer,
		float *depth_buffer)
{
	color4 new;

	if (ctx->colormask.ui) {
		if (ctx->flags & BLEND_ENABLE) {
			new = color_blend(*color_buffer, frag_color);
		} else {
			new = frag_color;
		}

		color_buffer->ui &= ~ctx->colormask.ui;
		color_buffer->ui |= new.ui & ctx->colormask.ui;
	}

	if (ctx->flags & DEPTH_WRITE)
		*depth_buffer = frag_depth;
}

static vec4 vertex_transform(const rs_vertex *in, context *ctx)
{
	float d, w = 1.0f / in->attribs[ATTRIB_POS].w;
	vec4 v;

	v = vec4_scale(in->attribs[ATTRIB_POS], w);
	d = (1.0f - v.z) * 0.5f;

	v.x = (1.0f + v.x) * 0.5f * (float)ctx->viewport.width +
		ctx->viewport.x;
	v.y = (1.0f - v.y) * 0.5f * (float)ctx->viewport.height +
		ctx->viewport.y;
	v.z = d * ctx->depth_far + (1.0f - d) * ctx->depth_near;
	v.w = w;

	return v;
}

static void vertex_prepare(rs_vertex *out, const rs_vertex *in, context *ctx)
{
	float d, w = 1.0f / in->attribs[ATTRIB_POS].w;
	int i, j;
	vec4 v;

	/* perspective divide of attributes */
	for (i = 0, j = 0x01; i < ATTRIB_COUNT; ++i, j <<= 1) {
		if (in->used & j)
			out->attribs[i] = vec4_scale(in->attribs[i], w);
	}

	/* viewport mapping */
	v = out->attribs[ATTRIB_POS];
	d = (1.0f - v.z) * 0.5f;

	v.x = (1.0f + v.x) * 0.5f * (float)ctx->viewport.width +
		ctx->viewport.x;
	v.y = (1.0f - v.y) * 0.5f * (float)ctx->viewport.height +
		ctx->viewport.y;
	v.z = d*ctx->depth_far + (1.0f - d)*ctx->depth_near;
	v.w = w;

	out->attribs[ATTRIB_POS] = v;
	out->used = in->used;
}

static int clip(const context *ctx, const vec4 A, const vec4 B, const vec4 C)
{
	if ((int)A.y > ctx->draw_area.maxy &&
		(int)B.y > ctx->draw_area.maxy &&
		(int)C.y > ctx->draw_area.maxy) {
		return 1;
	}
	if ((int)A.x > ctx->draw_area.maxx &&
		(int)B.x > ctx->draw_area.maxx &&
		(int)C.x > ctx->draw_area.maxx) {
		return 1;
	}
	if ((int)A.y < ctx->draw_area.miny &&
		(int)B.y < ctx->draw_area.miny &&
		(int)C.y < ctx->draw_area.miny) {
		return 1;
	}
	if ((int)A.x < ctx->draw_area.minx &&
		(int)B.x < ctx->draw_area.minx &&
		(int)C.x < ctx->draw_area.minx ) {
		return 1;
	}
	return 0;
}

static int cull(const context *ctx, const vec4 A, const vec4 B, const vec4 C)
{
	int ccw, cullccw = 0, cullcw = 0;

	ccw = ((C.x - A.x) * (C.y - B.y) - (C.y - A.y) * (C.x - B.x)) < 0.0f;

	if ((ctx->flags & (FRONT_CCW|CULL_FRONT)) == (FRONT_CCW|CULL_FRONT))
		cullccw = 1;
	if ((ctx->flags & (FRONT_CCW|CULL_BACK)) == CULL_BACK)
		cullccw = 1;

	if ((ctx->flags & (FRONT_CCW|CULL_BACK)) == (FRONT_CCW|CULL_BACK))
		cullcw = 1;
	if ((ctx->flags & (FRONT_CCW|CULL_FRONT)) == CULL_FRONT)
		cullcw = 1;

	return (ccw && cullccw) || (!ccw && cullcw);
}


static void per_pixel(context *ctx, const rs_vertex *v0,
			const rs_vertex *v1, const rs_vertex *v2)
{
	rs_vertex A, B, C;

	/* prepare vertices */
	vertex_prepare(&A, v0, ctx);
	vertex_prepare(&B, v1, ctx);
	vertex_prepare(&C, v2, ctx);

	/* clipping */
	if (clip(ctx, A.attribs[ATTRIB_POS], B.attribs[ATTRIB_POS],
			C.attribs[ATTRIB_POS])) {
		return;
	}

	/* culling */
	if (cull(ctx, A.attribs[ATTRIB_POS], B.attribs[ATTRIB_POS],
			C.attribs[ATTRIB_POS])) {
		return;
	}

	draw_triangle_per_pixel(&A, &B, &C, ctx);
}

void rasterizer_process_triangle(context *ctx, const rs_vertex *v0,
				const rs_vertex *v1, const rs_vertex *v2)
{
	if ((ctx->flags & CULL_FRONT) && (ctx->flags & CULL_BACK))
		return;

	if ((ctx->flags & DEPTH_TEST) && ctx->depth_test == COMPARE_NEVER)
		return;

	if (v0->attribs[ATTRIB_POS].w <= 0.0f ||
		v1->attribs[ATTRIB_POS].w <= 0.0f ||
		v2->attribs[ATTRIB_POS].w <= 0.0f) {
		return;
	}

	if (ctx->draw_area.minx >= ctx->draw_area.maxx ||
		ctx->draw_area.miny >= ctx->draw_area.maxy) {
		return;
	}

	per_pixel(ctx, v0, v1, v2);
}
