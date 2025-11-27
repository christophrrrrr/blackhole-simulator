#ifndef GRID_H
#define GRID_H

#include "math_utils.h"
#include "renderer.h"
#include <stdbool.h>

// grid visibility
extern bool is_grid_visible;

/**
 * @brief initialize grid buffers for double-buffering
 */
void grid_init_buffers(void);

/**
 * @brief cleanup grid buffers
 */
void grid_cleanup_buffers(void);

/**
 * @brief start the background grid generation thread
 */
void grid_start_thread(void);

/**
 * @brief stop the background grid generation thread
 */
void grid_stop_thread(void);

/**
 * @brief returns true if the grid thread is running
 */
bool grid_is_threaded(void);

/**
 * @brief update mesh from background thread (uploads to gpu)
 */
void grid_update_mesh(renderer_engine_t *engine);

/**
 * @brief legacy synchronous grid generation (fallback)
 */
void grid_generate_mesh(renderer_engine_t *engine);

/**
 * @brief render the grid
 */
void grid_render(renderer_engine_t *engine, matrix4_t view_projection_matrix);

#endif // GRID_H
