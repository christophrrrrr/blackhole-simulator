/**
spacetime grid visualization that is deformed based on the gravitational forces of the bodies
**/

#include "grid.h"
#include "physics.h"
#include "renderer.h"
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <time.h>
#include <stdatomic.h>
#include <string.h>

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#else
#include <GL/glew.h>
#endif

bool is_grid_visible = true;

// grid configuration
const int GRID_SIZE = 50;
const float GRID_SPACING = 1e10f;
const double PLANET_CURVATURE_SCALE = 500.0;

// grid mesh buffer structure
typedef struct
{
    vector3_t *vertices;
    unsigned int *indices;
    int vertex_count;
    int index_count;
} grid_buffer_t;

// ------------------------------
// internal threading primitives
// ------------------------------

static pthread_mutex_t grid_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t grid_thread_handle = 0;
static atomic_bool grid_thread_should_run = false;

// double-buffered grid data
static grid_buffer_t grid_buffers[2];
static int grid_read_buffer = 0;
static int grid_write_buffer = 1;
static atomic_bool grid_data_ready = false;

// ------------------------------
// grid generation core logic
// ------------------------------

static void compute_grid_vertices(grid_buffer_t *buffer)
{
    int vertex_count = 0;
    
    // snapshot physics data once
    celestial_body_t bodies_snapshot[NUM_CELESTIAL_BODIES];
    physics_lock();
    for (int i = 0; i < NUM_CELESTIAL_BODIES; ++i)
    {
        bodies_snapshot[i] = celestial_bodies[i];
    }
    physics_unlock();
    
    // compute grid vertices
    for (int z = 0; z <= GRID_SIZE; ++z)
    {
        for (int x = 0; x <= GRID_SIZE; ++x)
        {
            float world_x = (x - GRID_SIZE / 2) * GRID_SPACING;
            float world_z = (z - GRID_SIZE / 2) * GRID_SPACING;
            float y = -25e10f; // flat surface

            // for each celestial body
            for (int i = 0; i < NUM_CELESTIAL_BODIES; ++i)
            {
                // get position of the body
                vector3_t obj_pos = {
                    bodies_snapshot[i].position_and_radius.x,
                    bodies_snapshot[i].position_and_radius.y,
                    bodies_snapshot[i].position_and_radius.z
                };
                
                double mass = bodies_snapshot[i].mass;
                // calculate schwarzschild radius
                double schwarzschild_radius = 2.0 * GRAVITATIONAL_CONSTANT * mass / (SPEED_OF_LIGHT * SPEED_OF_LIGHT);
                
                // distance between grid point and body
                double dx = world_x - obj_pos.x;
                double dz = world_z - obj_pos.z;
                double dist_sq = dx * dx + dz * dz;

                double delta_y = 0.0;
                if (dist_sq > schwarzschild_radius * schwarzschild_radius)
                {
                    double dist = sqrt(dist_sq);

                    // visual approximation of spacetime curvature (flamm's paraboloid)
                    delta_y = sqrt(8.0 * schwarzschild_radius * (dist - schwarzschild_radius));

                    // non-black-hole objects have a different curvature scale
                    if (i != 2)
                    {
                        delta_y *= PLANET_CURVATURE_SCALE;
                    }
                }
                y += (float)delta_y;
            }
            buffer->vertices[vertex_count++] = (vector3_t){world_x, y, world_z};
        }
    }
    buffer->vertex_count = vertex_count;
}

static void compute_grid_indices(grid_buffer_t *buffer)
{
    int index_count = 0;
    
    // for each square add four lines (two edges)
    for (int z = 0; z < GRID_SIZE; ++z)
    {
        for (int x = 0; x < GRID_SIZE; ++x)
        {
            int i = z * (GRID_SIZE + 1) + x;
            
            // horizontal line
            buffer->indices[index_count++] = i;
            buffer->indices[index_count++] = i + 1;
            
            // vertical line
            buffer->indices[index_count++] = i;
            buffer->indices[index_count++] = i + GRID_SIZE + 1;
        }
    }
    buffer->index_count = index_count;
}

// ------------------------------
// background thread
// ------------------------------

static void* grid_thread_proc(void* arg)
{
    (void)arg;
    const double target_hz = 30.0; // grid updates at 30 Hz
    const long sleep_ns = (long)((1.0 / target_hz) * 1e9);
    
    while (atomic_load(&grid_thread_should_run))
    {
        if (!is_physics_paused)
        {
            // compute grid in the write buffer
            compute_grid_vertices(&grid_buffers[grid_write_buffer]);
            compute_grid_indices(&grid_buffers[grid_write_buffer]);
            
            // swap buffers
            pthread_mutex_lock(&grid_mutex);
            int temp = grid_read_buffer;
            grid_read_buffer = grid_write_buffer;
            grid_write_buffer = temp;
            atomic_store(&grid_data_ready, true);
            pthread_mutex_unlock(&grid_mutex);
        }
        
        // sleep until next update
        struct timespec req = {0, sleep_ns};
        nanosleep(&req, NULL);
    }
    return NULL;
}

// ------------------------------
// public api
// ------------------------------

void grid_init_buffers(void)
{
    int max_vertices = (GRID_SIZE + 1) * (GRID_SIZE + 1);
    int max_indices = GRID_SIZE * GRID_SIZE * 4;
    
    for (int i = 0; i < 2; ++i)
    {
        grid_buffers[i].vertices = malloc(max_vertices * sizeof(vector3_t));
        grid_buffers[i].indices = malloc(max_indices * sizeof(unsigned int));
        grid_buffers[i].vertex_count = 0;
        grid_buffers[i].index_count = 0;
    }
    
    // generate initial grid data
    compute_grid_vertices(&grid_buffers[grid_read_buffer]);
    compute_grid_indices(&grid_buffers[grid_read_buffer]);
    atomic_store(&grid_data_ready, true);
}

void grid_cleanup_buffers(void)
{
    for (int i = 0; i < 2; ++i)
    {
        free(grid_buffers[i].vertices);
        free(grid_buffers[i].indices);
        grid_buffers[i].vertices = NULL;
        grid_buffers[i].indices = NULL;
    }
}

void grid_start_thread(void)
{
    if (grid_thread_handle != 0)
        return;
    
    atomic_store(&grid_thread_should_run, true);
    if (pthread_create(&grid_thread_handle, NULL, grid_thread_proc, NULL) != 0)
    {
        atomic_store(&grid_thread_should_run, false);
        grid_thread_handle = 0;
    }
}

void grid_stop_thread(void)
{
    if (grid_thread_handle == 0)
        return;
    
    atomic_store(&grid_thread_should_run, false);
    pthread_join(grid_thread_handle, NULL);
    grid_thread_handle = 0;
}

bool grid_is_threaded(void)
{
    return grid_thread_handle != 0;
}

void grid_update_mesh(renderer_engine_t *engine)
{
    // check if new data is available
    if (!atomic_load(&grid_data_ready))
        return;
    
    // get the current read buffer
    pthread_mutex_lock(&grid_mutex);
    grid_buffer_t *buffer = &grid_buffers[grid_read_buffer];
    
    // initialize opengl buffers if needed
    if (engine->grid_vao == 0)
    {
        glGenVertexArrays(1, &engine->grid_vao);
        glGenBuffers(1, &engine->grid_vbo);
        glGenBuffers(1, &engine->grid_ebo);
    }
    
    // upload data to gpu
    glBindVertexArray(engine->grid_vao);
    glBindBuffer(GL_ARRAY_BUFFER, engine->grid_vbo);
    glBufferData(GL_ARRAY_BUFFER, buffer->vertex_count * sizeof(vector3_t), buffer->vertices, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, engine->grid_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, buffer->index_count * sizeof(unsigned int), buffer->indices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vector3_t), (void *)0);
    
    engine->grid_index_count = buffer->index_count;
    glBindVertexArray(0);
    
    pthread_mutex_unlock(&grid_mutex);
}

// legacy synchronous version (fallback when threading not used)
void grid_generate_mesh(renderer_engine_t *engine)
{
    grid_buffer_t temp_buffer;
    temp_buffer.vertices = malloc((GRID_SIZE + 1) * (GRID_SIZE + 1) * sizeof(vector3_t));
    temp_buffer.indices = malloc(GRID_SIZE * GRID_SIZE * 4 * sizeof(unsigned int));
    
    compute_grid_vertices(&temp_buffer);
    compute_grid_indices(&temp_buffer);
    
    if (engine->grid_vao == 0)
    {
        glGenVertexArrays(1, &engine->grid_vao);
        glGenBuffers(1, &engine->grid_vbo);
        glGenBuffers(1, &engine->grid_ebo);
    }
    
    glBindVertexArray(engine->grid_vao);
    glBindBuffer(GL_ARRAY_BUFFER, engine->grid_vbo);
    glBufferData(GL_ARRAY_BUFFER, temp_buffer.vertex_count * sizeof(vector3_t), temp_buffer.vertices, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, engine->grid_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, temp_buffer.index_count * sizeof(unsigned int), temp_buffer.indices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vector3_t), (void *)0);
    
    engine->grid_index_count = temp_buffer.index_count;
    glBindVertexArray(0);
    
    free(temp_buffer.vertices);
    free(temp_buffer.indices);
}

// draw the grid
void grid_render(renderer_engine_t *engine, matrix4_t view_projection_matrix)
{
    if (!is_grid_visible) return;

    glUseProgram(engine->grid_shader_program);
    glUniformMatrix4fv(glGetUniformLocation(engine->grid_shader_program, "viewProj"),
                       1, GL_FALSE, view_projection_matrix.elements);
    glBindVertexArray(engine->grid_vao);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDrawElements(GL_LINES, engine->grid_index_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}
