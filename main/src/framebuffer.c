#include "framebuffer.h"

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

void framebuffer_clear(framebuffer *fb, unsigned char r, unsigned char g,
			unsigned char b, unsigned char a)
{
	unsigned int i, count = fb->height * fb->width;
	unsigned char *ptr;

	for (ptr = fb->color, i = 0; i < count; ++i, ptr += 4) {
		ptr[RED] = r;
		ptr[GREEN] = g;
		ptr[BLUE] = b;
		ptr[ALPHA] = a;
	}
}

void framebuffer_clear_depth(framebuffer *fb, float value)
{
	unsigned int i, count = fb->height * fb->width;
	float *ptr;

	value = value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value);

	for (ptr = fb->depth, i = 0; i < count; ++i, ++ptr)
		*ptr = value;
}
