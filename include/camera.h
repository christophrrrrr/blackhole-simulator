#ifndef CAMERA_H
#define CAMERA_H

#include "math_utils.h"
#include <stdbool.h>

// camera state
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

vector3_t camera_get_position(const camera_t *cam);

void camera_update_moving_state(camera_t *cam);

void camera_process_mouse_move(camera_t *cam, double x, double y);

void camera_process_scroll(camera_t *cam, double y_offset);

void camera_reset(camera_t *cam);

#endif // CAMERA_H

