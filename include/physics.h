#ifndef PHYSICS_H
#define PHYSICS_H

#include "math_utils.h"
#include <stdbool.h>

// physical constants
extern const double SPEED_OF_LIGHT;
extern const double GRAVITATIONAL_CONSTANT;
extern const float BLACK_HOLE_SCHWARZSCHILD_RADIUS;
extern float RAY_INTEGRATION_STEP;
extern const double RAY_ESCAPE_RADIUS;
extern const int NUM_CELESTIAL_BODIES;
extern bool is_physics_paused;

// celestial body
typedef struct
{
    vector4_t position_and_radius; // .xyz for position, .w for radius
    vector4_t color;
    float mass;
    vector3_t velocity;
} celestial_body_t;

// global celestial bodies array
extern celestial_body_t celestial_bodies[];


void simulation_update_physics(double delta_time);

#endif // PHYSICS_H

