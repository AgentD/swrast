#ifndef COLOR_H
#define COLOR_H

#include <stdint.h>

#include "config.h"
#include "predef.h"
#include "vector.h"

union color4 {
	uint32_t ui;
	unsigned char components[4];
} __attribute__ ((aligned (4)));

static MATH_CONST color4 color_set(int r, int g, int b, int a)
{
	color4 col;
	col.components[RED] = r;
	col.components[GREEN] = g;
	col.components[BLUE] = b;
	col.components[ALPHA] = a;
	return col;
}

static MATH_CONST color4 color_from_vec(vec4 v)
{
	v = vec4_scale(v, 255.0f);
	return color_set(v.x, v.y, v.z, v.w);
}

static MATH_CONST color4 color_blend(color4 lower, color4 upper)
{
	uint32_t a, ia;
	color4 x;

	a = upper.components[ALPHA];
	ia = 0xFF - a;

	x.components[0] = (upper.components[0] * a +
				lower.components[0] * ia) >> 8;
	x.components[1] = (upper.components[1] * a +
				lower.components[1] * ia) >> 8;
	x.components[2] = (upper.components[2] * a +
				lower.components[2] * ia) >> 8;
	x.components[3] = (upper.components[3] * a +
				lower.components[3] * ia) >> 8;
	return x;
}

#endif /* COLOR_H */

