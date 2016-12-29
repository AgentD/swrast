/**
 * \file config.h
 *
 * \brief Contains preprocessor defined configuration of the rasterizer
 */
#ifndef CONFIG_H
#define CONFIG_H

#define MAX_TEXTURES 1
#define MAX_LIGHTS 8
#define FB_BGRA

#define MAX_INDEX_CACHE 31

#ifdef FB_BGRA
	#define RED 2
	#define GREEN 1
	#define BLUE 0
	#define ALPHA 3
#else
	#define RED 0
	#define GREEN 1
	#define BLUE 2
	#define ALPHA 3
#endif

#endif /* CONFIG_H */

