#ifndef VECTOR_H
#define VECTOR_H

#include "predef.h"
#include <math.h>

struct vec4 {
	float x;
	float y;
	float z;
	float w;
} __attribute__ ((aligned (16)));

static MATH_CONST vec4 vec4_set(float x, float y, float z, float w)
{
	vec4 v;
	v.x = x;
	v.y = y;
	v.z = z;
	v.w = w;
	return v;
}

static MATH_CONST vec4 vec4_mul(const vec4 a, const vec4 b)
{
	vec4 v;
	v.x = a.x * b.x;
	v.y = a.y * b.y;
	v.z = a.z * b.z;
	v.w = a.w * b.w;
	return v;
}

static MATH_CONST vec4 vec4_scale(const vec4 v, float s)
{
	vec4 out;
	out.x = v.x * s;
	out.y = v.y * s;
	out.z = v.z * s;
	out.w = v.w * s;
	return out;
}

static MATH_CONST vec4 vec4_invert(const vec4 o)
{
	vec4 out;
	out.x = -o.x;
	out.y = -o.y;
	out.z = -o.z;
	out.w = -o.w;
	return out;
}

static MATH_CONST vec4 vec4_sub(const vec4 a, const vec4 b)
{
	vec4 out;
	out.x = a.x - b.x;
	out.y = a.y - b.y;
	out.z = a.z - b.z;
	out.w = a.w - b.w;
	return out;
}

static MATH_CONST vec4 vec4_add(const vec4 a, const vec4 b)
{
	vec4 out;
	out.x = a.x + b.x;
	out.y = a.y + b.y;
	out.z = a.z + b.z;
	out.w = a.w + b.w;
	return out;
}

static MATH_CONST vec4 vec4_cross(const vec4 a, const vec4 b, float w)
{
	vec4 out;
	out.x = a.y * b.z - a.z * b.y;
	out.y = a.z * b.x - a.x * b.z;
	out.z = a.x * b.y - a.y * b.x;
	out.w = w;
	return out;
}

static MATH_CONST vec4 vec4_transform(const float *m, const vec4 in)
{
	vec4 out;

	out.x = m[0] * in.x + m[4] * in.y + m[ 8] * in.z + m[12] * in.w;
	out.y = m[1] * in.x + m[5] * in.y + m[ 9] * in.z + m[13] * in.w;
	out.z = m[2] * in.x + m[6] * in.y + m[10] * in.z + m[14] * in.w;
	out.w = m[3] * in.x + m[7] * in.y + m[11] * in.z + m[15] * in.w;

	return out;
}

static MATH_CONST float vec4_dot(const vec4 a, const vec4 b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

static MATH_CONST vec4 vec4_normalize(const vec4 v)
{
	float s = v.x * v.x + v.y * v.y + v.z * v.z;
	s = s > 0.0f ? 1.0f / sqrt(s) : 0.0f;
	return vec4_scale(v, s);
}

static MATH_CONST vec4 vec4_mix(const vec4 a, const vec4 b, float s)
{
	float inv = 1.0f - s;
	vec4 v;

	v.x = a.x * inv + b.x * s;
	v.y = a.y * inv + b.y * s;
	v.z = a.z * inv + b.z * s;
	v.w = a.w * inv + b.w * s;

	return v;
}

#endif /* VECTOR_H */

