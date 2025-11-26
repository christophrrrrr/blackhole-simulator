#ifndef GRID_H
#define GRID_H

#include "math_utils.h"
#include "renderer.h"
#include <stdbool.h>

// grid visibility
extern bool is_grid_visible;

void grid_generate_mesh(renderer_engine_t *engine);

void grid_render(renderer_engine_t *engine, matrix4_t view_projection_matrix);

#endif // GRID_H

