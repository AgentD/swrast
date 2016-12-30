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

static MATH_CONST vec4 color_to_vec(color4 c)
{
	return vec4_set((float)c.components[RED] / 255.0f,
			(float)c.components[GREEN] / 255.0f,
			(float)c.components[BLUE] / 255.0f,
			(float)c.components[ALPHA] / 255.0f);
}

static MATH_CONST int color_red(color4 c)
{
	return c.components[RED];
}

static MATH_CONST int color_green(color4 c)
{
	return c.components[GREEN];
}

static MATH_CONST int color_blue(color4 c)
{
	return c.components[BLUE];
}

static MATH_CONST int color_alpha(color4 c)
{
	return c.components[ALPHA];
}

#endif /* COLOR_H */

