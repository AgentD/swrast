#include "inputassembler.h"
#include "rasterizer.h"
#include "context.h"
#include "config.h"
#include "shader.h"
#include "vector.h"
#include <math.h>

static MATH_CONST vec4 decode_f2(void *ptr)
{
	return vec4_set(((float *)ptr)[0], ((float *)ptr)[1], 0.0f, 1.0f);
}

static MATH_CONST vec4 decode_f3(void *ptr)
{
	return vec4_set(((float *)ptr)[0], ((float *)ptr)[1],
			((float *)ptr)[2], 1.0f);
}

static MATH_CONST vec4 decode_f4(void *ptr)
{
	return vec4_set(((float *)ptr)[0], ((float *)ptr)[1],
			((float *)ptr)[2], ((float *)ptr)[3]);
}

static unsigned char *read_vertex(rs_vertex *v, unsigned char *ptr,
				int vertex_format)
{
	/* initialize vertex structure */
	v->attribs[ATTRIB_POS] = vec4_set(0.0f, 0.0f, 0.0f, 1.0f);
	v->attribs[ATTRIB_COLOR] = vec4_set(1.0f, 1.0f, 1.0f, 1.0f);
	v->attribs[ATTRIB_NORMAL] = vec4_set(0.0f, 0.0f, 0.0f, 0.0f);
	v->attribs[ATTRIB_TEX0] = vec4_set(0.0f, 0.0f, 0.0f, 1.0f);
	v->attribs[ATTRIB_TEX1] = vec4_set(0.0f, 0.0f, 0.0f, 1.0f);
	v->used = 0;

	/* decode position */
	if (vertex_format & (VF_POSITION_F2|VF_POSITION_F3|VF_POSITION_F4))
		v->used |= ATTRIB_FLAG_POS;

	if (vertex_format & VF_POSITION_F2) {
		v->attribs[ATTRIB_POS] = decode_f2(ptr);
		ptr += 2 * sizeof(float);
	} else if (vertex_format & VF_POSITION_F3) {
		v->attribs[ATTRIB_POS] = decode_f3(ptr);
		ptr += 3 * sizeof(float);
	} else if (vertex_format & VF_POSITION_F4) {
		v->attribs[ATTRIB_POS] = decode_f4(ptr);
		ptr += 4 * sizeof(float);
	}

	/* decode surface normal */
	if (vertex_format & VF_NORMAL_F3) {
		v->used |= ATTRIB_FLAG_NORMAL;
		v->attribs[ATTRIB_NORMAL] = decode_f3(ptr);
		ptr += 3 * sizeof(float);
	}

	/* decode color */
	if (vertex_format & (VF_COLOR_F3|VF_COLOR_F4|VF_COLOR_UB3|VF_COLOR_UB4))
		v->used |= ATTRIB_FLAG_COLOR;

	if (vertex_format & VF_COLOR_F3) {
		v->attribs[ATTRIB_COLOR] = decode_f3(ptr);
		ptr += 3 * sizeof(float);
	} else if (vertex_format & VF_COLOR_F4) {
		v->attribs[ATTRIB_COLOR] = decode_f4(ptr);
		ptr += 4 * sizeof(float);
	} else if (vertex_format & VF_COLOR_UB3) {
		v->attribs[ATTRIB_COLOR] = vec4_set(((float)ptr[0])/255.0f,
						((float)ptr[1])/255.0f,
						((float)ptr[2])/255.0f,
						1.0f);
		ptr += 3;
	} else if (vertex_format & VF_COLOR_UB4) {
		v->attribs[ATTRIB_COLOR] = vec4_set(((float)ptr[0])/255.0f,
						((float)ptr[1])/255.0f,
						((float)ptr[2])/255.0f,
						((float)ptr[3])/255.0f);
		ptr += 4;
	}

	/* decode texture coordinates */
	if (vertex_format & VF_TEX0) {
		v->used |= ATTRIB_FLAG_TEX0;
		v->attribs[ATTRIB_TEX0] = decode_f2(ptr);
		ptr += 2*sizeof(float);
	}
	return ptr;
}

static void draw_triangle(context *ctx, rs_vertex *v0, rs_vertex *v1,
			rs_vertex *v2)
{
	shader_process_vertex(ctx, v0);
	shader_process_vertex(ctx, v1);
	shader_process_vertex(ctx, v2);

	rasterizer_process_triangle(ctx, v0, v1, v2);
}

static void invalidate_tl_cache(context *ctx)
{
	int i;

	for (i = 0; i < MAX_INDEX_CACHE; ++i)
		ctx->post_tl_cache[i].index = -1;
}

static void get_cached_index(context *ctx, rs_vertex *v,
				unsigned int vsize, unsigned int i)
{
	int idx, slot = i % MAX_INDEX_CACHE;

	idx = ctx->post_tl_cache[slot].index;

	if (idx > 0 && (unsigned int)idx == i) {
		*v = ctx->post_tl_cache[slot].vtx;
	} else {
		read_vertex(v,
			((unsigned char *)ctx->vertexbuffer) + vsize * i,
			ctx->vertex_format);

		shader_process_vertex(ctx, v);

		ctx->post_tl_cache[slot].vtx = *v;
		ctx->post_tl_cache[slot].index = i;
	}
}

static void draw_triangle_indexed(context *ctx, unsigned int vsize,
				unsigned int i0, unsigned int i1,
				unsigned int i2)
{
	rs_vertex v0, v1, v2;

	get_cached_index(ctx, &v0, vsize, i0);
	get_cached_index(ctx, &v1, vsize, i1);
	get_cached_index(ctx, &v2, vsize, i2);

	rasterizer_process_triangle(ctx, &v0, &v1, &v2);
}

void ia_draw_triangles(context *ctx, unsigned int vertexcount)
{
	void *ptr = ctx->vertexbuffer;
	rs_vertex v0, v1, v2;
	unsigned int i;

	if (ctx->immediate.active)
		return;

	vertexcount -= vertexcount % 3;

	for (i = 0; i < vertexcount; i += 3) {
		ptr = read_vertex(&v0, ptr, ctx->vertex_format);
		ptr = read_vertex(&v1, ptr, ctx->vertex_format);
		ptr = read_vertex(&v2, ptr, ctx->vertex_format);

		draw_triangle(ctx, &v0, &v1, &v2);
	}
}

void ia_draw_triangles_indexed(context *ctx, unsigned int vertexcount,
				unsigned int indexcount)
{
	unsigned int i0, i1, i2, i = 0, vsize = 0;

	if (ctx->immediate.active)
		return;

	/* determine vertex size in bytes */
	if (ctx->vertex_format & VF_POSITION_F2) {
		vsize += 2 * sizeof(float);
	} else if (ctx->vertex_format & VF_POSITION_F3) {
		vsize += 3 * sizeof(float);
	} else if (ctx->vertex_format & VF_POSITION_F4) {
		vsize += 4 * sizeof(float);
	}

	if (ctx->vertex_format & VF_NORMAL_F3)
		vsize += 3 * sizeof(float);

	if (ctx->vertex_format & VF_COLOR_F3) {
		vsize += 3 * sizeof(float);
	} else if (ctx->vertex_format & VF_COLOR_F4) {
		vsize += 4 * sizeof(float);
	} else if (ctx->vertex_format & VF_COLOR_UB3) {
		vsize += 3;
	} else if (ctx->vertex_format & VF_COLOR_UB4) {
		vsize += 4;
	}

	if (ctx->vertex_format & VF_TEX0)
		vsize += 2 * sizeof(float);

	/* for each triangle */
	invalidate_tl_cache(ctx);

	indexcount -= indexcount % 3;

	while (i < indexcount) {
		i0 = ctx->indexbuffer[i++];
		i1 = ctx->indexbuffer[i++];
		i2 = ctx->indexbuffer[i++];

		if (i0 < vertexcount && i1 < vertexcount && i2 < vertexcount)
			draw_triangle_indexed(ctx, vsize, i0, i1, i2);
	}
}

void ia_begin(context *ctx)
{
	ctx->immediate.next.used = 0;
	ctx->immediate.current = 0;
	ctx->immediate.active = 1;
}

void ia_vertex(context *ctx, float x, float y, float z, float w)
{
	if (!ctx->immediate.active)
		return;

	ctx->immediate.next.attribs[ATTRIB_POS] = vec4_set(x, y, z, w);
	ctx->immediate.next.used |= ATTRIB_FLAG_POS;

	ctx->immediate.vertex[ctx->immediate.current++] = ctx->immediate.next;

	if (ctx->immediate.current < 3)
		return;

	ctx->immediate.current = 0;
	draw_triangle(ctx, ctx->immediate.vertex, ctx->immediate.vertex + 1,
			ctx->immediate.vertex + 2);
}

void ia_color(context *ctx, float r, float g, float b, float a)
{
	ctx->immediate.next.attribs[ATTRIB_COLOR] = vec4_set(r, g, b, a);
	ctx->immediate.next.used |= ATTRIB_FLAG_COLOR;
}

void ia_normal(context *ctx, float x, float y, float z)
{
	ctx->immediate.next.attribs[ATTRIB_NORMAL] = vec4_set(x, y, z, 0.0f);
	ctx->immediate.next.used |= ATTRIB_FLAG_NORMAL;
}

void ia_texcoord(context *ctx, int layer, float s, float t)
{
	ctx->immediate.next.attribs[ATTRIB_TEX0 + layer] =
		vec4_set(s, t, 0.0f, 1.0f);

	ctx->immediate.next.used |= (ATTRIB_FLAG_TEX0 << layer);
}

void ia_end(context *ctx)
{
	ctx->immediate.next.used = 0;
	ctx->immediate.current = 0;
	ctx->immediate.active = 0;
}
