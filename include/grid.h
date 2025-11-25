/**
 * @file grid.h
 * @brief spacetime grid visualization
 */

#ifndef GRID_H
#define GRID_H

#include "math_utils.h"
#include "renderer.h"
#include <stdbool.h>

// grid visibility
extern bool is_grid_visible;

/**
 * @brief generates or updates the vertex data for the spacetime grid visualization.
 * the grid is deformed based on the gravitational potential of the celestial bodies.
 */
void grid_generate_mesh(renderer_engine_t *engine);

/**
 * @brief renders the spacetime grid.
 */
void grid_render(renderer_engine_t *engine, matrix4_t view_projection_matrix);

#endif // GRID_H

