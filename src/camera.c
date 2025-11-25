/**
 * @file camera.c
 * @brief implementation of the camera system
 */

#include "camera.h"
#include "math_utils.h"
#include <math.h>

const camera_t initial_camera_state = {
    .target = {0.0f, 0.0f, 0.0f},
    .radius = 17.0e10f,
    .min_radius = 1e10f,
    .max_radius = 25.0e10f,
    .azimuth = 0.0f,
    .elevation = M_PI / 2.4f,
    .orbit_speed = 0.01f,
    .pan_speed = 0.005f,
    .zoom_speed = 25e9f,
    .is_dragging_orbit = false,
    .is_dragging_pan = false,
    .is_moving = false,
    .last_cursor_x = 0.0,
    .last_cursor_y = 0.0};

camera_t camera;

vector3_t camera_get_position(const camera_t *cam)
{
    // clamp elevation to avoid gimbal lock at the poles
    float clamped_elevation = utility_clamp_float(cam->elevation, 0.01f, M_PI - 0.01f);
    vector3_t orbital_pos = {
        cam->radius * sinf(clamped_elevation) * cosf(cam->azimuth),
        cam->radius * cosf(clamped_elevation),
        cam->radius * sinf(clamped_elevation) * sinf(cam->azimuth)};
    return vector3_add(cam->target, orbital_pos);
}

void camera_update_moving_state(camera_t *cam)
{
    cam->is_moving = cam->is_dragging_orbit || cam->is_dragging_pan;
}

void camera_process_mouse_move(camera_t *cam, double x, double y)
{
    float dx = (float)(x - cam->last_cursor_x);
    float dy = (float)(y - cam->last_cursor_y);

    if (cam->is_dragging_orbit)
    {
        cam->azimuth += dx * cam->orbit_speed;
        cam->elevation -= dy * cam->orbit_speed;
        cam->elevation = utility_clamp_float(cam->elevation, 0.01f, M_PI - 0.01f);
    }

    if (cam->is_dragging_pan)
    {
        vector3_t pos = camera_get_position(cam);
        vector3_t fwd = vector3_normalize(vector3_subtract(cam->target, pos));
        vector3_t global_up = {0, 1, 0};
        vector3_t right = vector3_normalize(vector3_cross(fwd, global_up));
        vector3_t up = vector3_normalize(vector3_cross(right, fwd));

        // scale pan speed with distance to maintain consistent feel
        float pan_scale = cam->radius / 1e11f;
        vector3_t pan_offset_x = vector3_scale(right, -dx * cam->pan_speed * pan_scale);
        vector3_t pan_offset_y = vector3_scale(up, dy * cam->pan_speed * pan_scale);
        cam->target = vector3_add(cam->target, pan_offset_x);
        cam->target = vector3_add(cam->target, pan_offset_y);
    }

    cam->last_cursor_x = x;
    cam->last_cursor_y = y;
    camera_update_moving_state(cam);
}

void camera_process_scroll(camera_t *cam, double y_offset)
{
    cam->radius -= (float)(y_offset * cam->zoom_speed);
    cam->radius = utility_clamp_float(cam->radius, cam->min_radius, cam->max_radius);
    camera_update_moving_state(cam);
}

void camera_reset(camera_t *cam)
{
    *cam = initial_camera_state;
}

