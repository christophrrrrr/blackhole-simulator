/**
 * @file physics.c
 * @brief implementation of physics simulation
 */

#include "physics.h"
#include <math.h>

// simulation and physical constants
const double SPEED_OF_LIGHT = 299792458.0;
const double GRAVITATIONAL_CONSTANT = 6.67430e-11;
const float BLACK_HOLE_SCHWARZSCHILD_RADIUS = 1.269e10f;
float RAY_INTEGRATION_STEP = 5e7f; // initial step size for ray integration
const double RAY_ESCAPE_RADIUS = 1e30;    // radius at which rays are considered to have escaped
const int NUM_CELESTIAL_BODIES = 3;
bool is_physics_paused = false;

celestial_body_t celestial_bodies[] = {
    {{2.3e11f, 0.0f, 0.0f, 4e10f},   // position and radius
     {0.4, 0.7, 1.0, 1.0},           // color (blue star)
     1.98892e30f,                    // mass (solar mass)
     {0.0f, 0.0f, 5.34e7}},          // initial velocity
    {{-1.6e11f, 0.0f, 0.0f, 4e10f},  // position and radius
     {0.8, 0.3, 0.2, 1.0},           // color (red star)
     1.98892e30f,                    // mass (solar mass)
     {0.0f, 0.0f, -5.34e7}},         // initial velocity
    {{0.0f, 0.0f, 0.0f, BLACK_HOLE_SCHWARZSCHILD_RADIUS}, // position and radius
     {0, 0, 0, 1},                   // color (black hole)
     8.54e36f,                       // mass (supermassive)
     {0, 0, 0}}                      // initial velocity
};

void simulation_update_physics(double delta_time)
{
    if (is_physics_paused)
        return;

    // n-body simulation using newton's law of universal gravitation.
    for (int i = 0; i < NUM_CELESTIAL_BODIES; ++i)
    {
        for (int j = 0; j < NUM_CELESTIAL_BODIES; ++j)
        {
            if (i == j)
                continue;

            // calculate distance and direction between bodies
            float dx = celestial_bodies[j].position_and_radius.x - celestial_bodies[i].position_and_radius.x;
            float dy = celestial_bodies[j].position_and_radius.y - celestial_bodies[i].position_and_radius.y;
            float dz = celestial_bodies[j].position_and_radius.z - celestial_bodies[i].position_and_radius.z;
            float distance = sqrtf(dx * dx + dy * dy + dz * dz);

            // avoid division by zero and collision
            if (distance > (celestial_bodies[i].position_and_radius.w + celestial_bodies[j].position_and_radius.w))
            {
                vector3_t direction = {dx / distance, dy / distance, dz / distance};
                double gravitational_force = (GRAVITATIONAL_CONSTANT * celestial_bodies[i].mass * celestial_bodies[j].mass) / (distance * distance);
                double acceleration = gravitational_force / celestial_bodies[i].mass;

                // update velocity based on acceleration
                celestial_bodies[i].velocity.x += direction.x * acceleration * delta_time;
                celestial_bodies[i].velocity.y += direction.y * acceleration * delta_time;
                celestial_bodies[i].velocity.z += direction.z * acceleration * delta_time;
            }
        }
    }
    // update positions based on new velocities
    for (int i = 0; i < NUM_CELESTIAL_BODIES; i++)
    {
        celestial_bodies[i].position_and_radius.x += celestial_bodies[i].velocity.x * delta_time;
        celestial_bodies[i].position_and_radius.y += celestial_bodies[i].velocity.y * delta_time;
        celestial_bodies[i].position_and_radius.z += celestial_bodies[i].velocity.z * delta_time;
    }
}

