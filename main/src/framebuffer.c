#include "framebuffer.h"
#include "color.h"

#include <stdlib.h>

int framebuffer_init(framebuffer *fb, unsigned int width, unsigned int height)
{
	fb->width = width;
	fb->height = height;

	fb->color = malloc(width * height * 4);

	if (!fb->color)
		return 0;

	fb->depth = malloc(width * height * sizeof(float));

	if (!fb->depth) {
		free(fb->color);
		return 0;
	}
	return 1;
}

void framebuffer_cleanup(framebuffer *fb)
{
	free(fb->depth);
	free(fb->color);
}

void framebuffer_clear(framebuffer *fb, int r, int g, int b, int a)
{
	color4 *ptr = fb->color, val = color_set(r, g, b, a);
	unsigned int i, count = fb->height * fb->width;

	for (i = 0; i < count; ++i)
		*(ptr++) = val;
}

void framebuffer_clear_depth(framebuffer *fb, float value)
{
	unsigned int i, count = fb->height * fb->width;
	float *ptr;

	value = value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value);

	for (ptr = fb->depth, i = 0; i < count; ++i, ++ptr)
		*ptr = value;
}
