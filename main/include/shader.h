/**
 * \file shader.h
 *
 * \brief Contians shader functions
 */
#ifndef SHADER_H
#define SHADER_H

#include "predef.h"

/**
 * \enum SHADER_PROGRAM
 *
 * \brief Shader program enumerator
 */
typedef enum {
	/**
	 * \brief Do not do any lighting calulation, simply use interpolated
	 *        vertex colors and apply texture colors
	 */
	SHADER_UNLIT = 0,

	/** \brief Compute colors based on blinn-phong lighting model */
	SHADER_PHONG = 1
} SHADER_PROGRAM;

/**
 * \interface shader_program
 *
 * \brief Abstracts entry points of a shader program
 */
struct shader_program {
	/**
	 * \brief Run the vertex shader on a vertex
	 *
	 * \param prog A pointer to the program itself
	 * \param ctx  A pointer to a context
	 * \param vert A pointer to a vertex to process
	 */
	void(* vertex )(const shader_program *prog,
			const context *ctx, rs_vertex *vert);

	/**
	 * \brief Run the frament shader on the interpolated vertex attributes
	 *
	 * \param prog A pointer to the program itself
	 * \param ctx  A pointer to a context
	 * \param frag A pointer to a the interpolated vertex attributes
	 *
	 * \return A color value for the fragment
	 */
	vec4(* fragment )(const shader_program *prog,
			const context *ctx, const rs_vertex *frag);
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Get a pointer to an internal shader program by ID
 *
 * \param id A \ref SHADER_PROGRAM identifier
 *
 * \return A shader on success, NULL on failure
 */
const shader_program *shader_internal(unsigned int id);

/**
 * \brief Helper function for shaders to compute lighting values
 *
 * Computes the color resulting from the blinn-phong ilumination model for a
 * light index in the context.
 *
 * \param ctx The context to take the light and material parameters from
 * \param i   The light index, i.e. which light from the context to use
 * \param V   A vector pointing from the surface towards the viewer
 * \param N   The normal vector at the specified surface location
 *
 * \return The resulting color value
 */
vec4 blinn_phong(const context *ctx, int i, const vec4 V, const vec4 N);

#ifdef __cplusplus
}
#endif

#endif /* SHADER_H */

