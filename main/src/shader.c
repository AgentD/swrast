#include "rasterizer.h"
#include "context.h"
#include "texture.h"
#include "shader.h"

#include <math.h>
#include <stddef.h>

vec4 blinn_phong(const context *ctx, int i, const vec4 V, const vec4 N)
{
	float dist, att, ks, kd;
	vec4 L, H, cd, cs;

	/* light vector */
	L = vec4_add(ctx->light[i].position, V);
	L.w = 0.0f;

	dist = sqrt(vec4_dot(L, L));
	L = vec4_scale(L, dist > 0.0f ? 1.0f / dist : 0.0f);

	/* half vector */
	H = vec4_normalize(vec4_add(L, V));

	/* attenuation factor */
	att = ctx->light[i].attenuation_constant +
		ctx->light[i].attenuation_linear * dist +
		ctx->light[i].attenuation_quadratic * dist * dist;
	att = att > 0.0f ? 1.0f / att : 0.0f;

	/* diffuse component */
	kd = vec4_dot(N, L);
	kd = kd < 0.0f ? 0.0f : kd;

	/* specular component */
	ks = vec4_dot(N, H);
	ks = ks < 0.0f ? 0.0f : ks;
	ks = pow(ks, ctx->material.shininess);

	/* combine */
	cd = vec4_mul(ctx->light[i].diffuse, ctx->material.diffuse);
	cs = vec4_mul(ctx->light[i].specular, ctx->material.specular);

	return vec4_add(vec4_scale(cd, kd * att), vec4_scale(cs, ks * att));
}

static void mv_transform(const context *ctx, rs_vertex *v)
{
	/* transform normal to viewspace */
	v->attribs[ATTRIB_NORMAL] = vec4_transform(ctx->normalmatrix,
						v->attribs[ATTRIB_NORMAL]);

	/* transform position to viewspace */
	v->attribs[ATTRIB_POS] = vec4_transform(ctx->modelview,
						v->attribs[ATTRIB_POS]);
}

static vec4 apply_textures(const context *ctx, const rs_vertex *frag)
{
	vec4 tex, c = { 1.0f, 1.0f, 1.0f, 1.0f };
	int i;

	for (i = 0; i < MAX_TEXTURES; ++i) {
		if (!ctx->texture_enable[i])
			continue;

		tex = texture_sample(ctx->textures[i],
					frag->attribs[ATTRIB_TEX0 + i]);
		c = vec4_mul(c, tex);
	}

	return c;
}

/****************************************************************************/

static void shader_unlit_vertex(const shader_program *prog, const context *ctx,
				rs_vertex *v)
{
	vec4 V = vec4_transform(ctx->modelview, v->attribs[ATTRIB_POS]);
	(void)prog;

	v->attribs[ATTRIB_POS] = vec4_transform(ctx->projection, V);
	v->used &= ~(ATTRIB_FLAG_NORMAL|ATTRIB_FLAG_USR0|ATTRIB_FLAG_USR1);
}

static vec4 shader_unlit_fragment(const shader_program *prog,
				const context *ctx, const rs_vertex *frag)
{
	(void)prog;
	return vec4_mul(apply_textures(ctx, frag),
			frag->attribs[ATTRIB_COLOR]);
}

/****************************************************************************/

static void shader_phong_vertex(const shader_program *prog, const context *ctx,
				rs_vertex *vert)
{
	vec4 V;
	int i;
	(void)prog;

	mv_transform(ctx, vert);

	V = vec4_invert(vert->attribs[ATTRIB_POS]);
	V.w = 0.0f;
	vert->attribs[ATTRIB_USR0] = V;

	V = vec4_set(0.0f, 0.0f, 0.0f, 0.0f);

	for (i = 0; i < MAX_LIGHTS; ++i) {
		if (!ctx->light[i].enable)
			continue;
		V = vec4_mul(ctx->light[i].ambient, ctx->material.ambient);
	}

	vert->attribs[ATTRIB_USR1] = vec4_add(V, ctx->material.emission);

	vert->used |= ATTRIB_FLAG_USR0|ATTRIB_FLAG_USR1;

	vert->attribs[ATTRIB_POS] = vec4_transform(ctx->projection,
						vert->attribs[ATTRIB_POS]);
}

static vec4 shader_phong_fragment(const shader_program *prog,
				const context *ctx, const rs_vertex *frag)
{
	vec4 color, V, N;
	int i;
	(void)prog;

	color = frag->attribs[ATTRIB_USR1];
	V = vec4_normalize(frag->attribs[ATTRIB_USR0]);
	N = vec4_normalize(frag->attribs[ATTRIB_NORMAL]);

	for (i = 0; i < MAX_LIGHTS; ++i) {
		if (ctx->light[i].enable)
			color = vec4_add(color, blinn_phong(ctx, i, V, N));
	}

	color.w = 1.0f;

	if (frag->used & ATTRIB_FLAG_COLOR)
		color = vec4_mul(frag->attribs[ATTRIB_COLOR], color);

	return vec4_mul(apply_textures(ctx, frag), color);
}

/****************************************************************************/

static const shader_program shaders[] = {
	{ shader_unlit_vertex, shader_unlit_fragment },
	{ shader_phong_vertex, shader_phong_fragment },
};

const shader_program *shader_internal(unsigned int id)
{
	return id < sizeof(shaders)/sizeof(shaders[0]) ? shaders + id : NULL;
}
