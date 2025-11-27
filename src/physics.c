/**
 * @file physics.c
 * @brief implementation of physics simulation
 */

#include "physics.h"
#include <math.h>
#include <pthread.h>
#include <time.h>
#include <stdatomic.h>

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

// ------------------------------
// Internal threading primitives
// ------------------------------

static pthread_mutex_t physics_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t physics_thread_handle = 0;
static atomic_bool physics_thread_should_run = false;

// Next-state buffer for threaded stepping (not exposed)
static celestial_body_t celestial_bodies_next[NUM_CELESTIAL_BODIES];

static void simulation_step_buffered(const celestial_body_t *in_bodies, celestial_body_t *out_bodies, double delta_time)
{
    // copy input to output to ensure we keep unmodified fields if needed
    for (int k = 0; k < NUM_CELESTIAL_BODIES; ++k)
    {
        out_bodies[k] = in_bodies[k];
    }

    // n-body simulation using newton's law of universal gravitation.
    for (int i = 0; i < NUM_CELESTIAL_BODIES; ++i)
    {
        double vx = in_bodies[i].velocity.x;
        double vy = in_bodies[i].velocity.y;
        double vz = in_bodies[i].velocity.z;

        for (int j = 0; j < NUM_CELESTIAL_BODIES; ++j)
        {
            if (i == j)
                continue;

            float dx = in_bodies[j].position_and_radius.x - in_bodies[i].position_and_radius.x;
            float dy = in_bodies[j].position_and_radius.y - in_bodies[i].position_and_radius.y;
            float dz = in_bodies[j].position_and_radius.z - in_bodies[i].position_and_radius.z;
            float distance = sqrtf(dx * dx + dy * dy + dz * dz);

            if (distance > (in_bodies[i].position_and_radius.w + in_bodies[j].position_and_radius.w))
            {
                vector3_t direction = {dx / distance, dy / distance, dz / distance};
                double gravitational_force = (GRAVITATIONAL_CONSTANT * in_bodies[i].mass * in_bodies[j].mass) / (distance * distance);
                double acceleration = gravitational_force / in_bodies[i].mass;
                vx += direction.x * acceleration * delta_time;
                vy += direction.y * acceleration * delta_time;
                vz += direction.z * acceleration * delta_time;
            }
        }

        out_bodies[i].velocity.x = (float)vx;
        out_bodies[i].velocity.y = (float)vy;
        out_bodies[i].velocity.z = (float)vz;
    }

    for (int i = 0; i < NUM_CELESTIAL_BODIES; i++)
    {
        out_bodies[i].position_and_radius.x += out_bodies[i].velocity.x * (float)delta_time;
        out_bodies[i].position_and_radius.y += out_bodies[i].velocity.y * (float)delta_time;
        out_bodies[i].position_and_radius.z += out_bodies[i].velocity.z * (float)delta_time;
    }
}

void simulation_update_physics(double delta_time)
{
    if (is_physics_paused)
        return;

    // In-place update for single-threaded mode
    simulation_step_buffered(celestial_bodies, celestial_bodies, delta_time);
}

// ---------------
// Thread control
// ---------------

void physics_lock(void)
{
    pthread_mutex_lock(&physics_mutex);
}

void physics_unlock(void)
{
    pthread_mutex_unlock(&physics_mutex);
}

bool physics_is_threaded(void)
{
    return physics_thread_handle != 0;
}

static void* physics_thread_proc(void* arg)
{
    (void)arg;
    const double target_hz = 60.0;
    const double sim_speed = 500.0;
    const long sleep_ns = (long)((1.0 / target_hz) * 1e9);

    while (atomic_load(&physics_thread_should_run))
    {
        if (!is_physics_paused)
        {
            simulation_step_buffered(celestial_bodies, celestial_bodies_next, (1.0 / target_hz) * sim_speed);
            physics_lock();
            for (int i = 0; i < NUM_CELESTIAL_BODIES; ++i)
            {
                celestial_bodies[i] = celestial_bodies_next[i];
            }
            physics_unlock();
        }
        
        // cross-platform sleep using nanosleep
        struct timespec req = {0, sleep_ns};
        nanosleep(&req, NULL);
    }
    return NULL;
}

void physics_start_thread(void)
{
    if (physics_thread_handle != 0)
        return;
    
    atomic_store(&physics_thread_should_run, true);
    if (pthread_create(&physics_thread_handle, NULL, physics_thread_proc, NULL) != 0)
    {
        atomic_store(&physics_thread_should_run, false);
        physics_thread_handle = 0;
    }
}

void physics_stop_thread(void)
{
    if (physics_thread_handle == 0)
        return;
    
    atomic_store(&physics_thread_should_run, false);
    pthread_join(physics_thread_handle, NULL);
    physics_thread_handle = 0;
}

