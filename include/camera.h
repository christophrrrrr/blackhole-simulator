/**
 * @file camera.h
 * @brief interactive orbital camera system
 */

#ifndef CAMERA_H
#define CAMERA_H

#include "math_utils.h"
#include <stdbool.h>

/**
 * @struct camera_t
 * @brief represents the state of the interactive orbital camera.
 */
typedef struct
{
    vector3_t target;
    float radius;
    float min_radius, max_radius;
    float azimuth;
    float elevation;
    float orbit_speed;
    float pan_speed;
    double zoom_speed;
    bool is_dragging_orbit;
    bool is_dragging_pan;
    bool is_moving;
    double last_cursor_x, last_cursor_y;
} camera_t;

// global camera state
extern const camera_t initial_camera_state;
extern camera_t camera;

/**
 * @brief calculates the camera's world position based on its orbital parameters.
 */
vector3_t camera_get_position(const camera_t *cam);

/**
 * @brief updates the camera's moving state.
 */
void camera_update_moving_state(camera_t *cam);

/**
 * @brief processes mouse movement to update camera orientation and position.
 */
void camera_process_mouse_move(camera_t *cam, double x, double y);

/**
 * @brief processes mouse scroll events to zoom the camera.
 */
void camera_process_scroll(camera_t *cam, double y_offset);

/**
 * @brief resets the camera to its default state.
 */
void camera_reset(camera_t *cam);

#endif // CAMERA_H

