/**
 * @file grid.c
 * @brief implementation of spacetime grid rendering
 */

#include "grid.h"
#include "physics.h"
#include "renderer.h"
#include <stdlib.h>
#include <math.h>

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#else
#include <GL/glew.h>
#endif

bool is_grid_visible = true;

void grid_generate_mesh(renderer_engine_t *engine)
{
    const int grid_size = 50;
    const float grid_spacing = 1e10f;
    const double planet_curvature_scale = 500.0; // scaling for visual effect

    vector3_t *vertices = malloc((grid_size + 1) * (grid_size + 1) * sizeof(vector3_t));
    GLuint *indices = malloc(grid_size * grid_size * 4 * sizeof(GLuint));

    int vertex_count = 0;
    int index_count = 0;

	physics_lock();
    for (int z = 0; z <= grid_size; ++z)
    {
        for (int x = 0; x <= grid_size; ++x)
        {
            float world_x = (x - grid_size / 2) * grid_spacing;
            float world_z = (z - grid_size / 2) * grid_spacing;
            float y = -25e10f;

            for (int i = 0; i < NUM_CELESTIAL_BODIES; ++i)
            {
                vector3_t obj_pos = {celestial_bodies[i].position_and_radius.x, celestial_bodies[i].position_and_radius.y, celestial_bodies[i].position_and_radius.z};
                double mass = celestial_bodies[i].mass;
                double schwarzschild_radius = 2.0 * GRAVITATIONAL_CONSTANT * mass / (SPEED_OF_LIGHT * SPEED_OF_LIGHT);
                double dx = world_x - obj_pos.x;
                double dz = world_z - obj_pos.z;
                double dist_sq = dx * dx + dz * dz;

                double delta_y = 0.0;
                if (dist_sq > schwarzschild_radius * schwarzschild_radius)
                {
                    double dist = sqrt(dist_sq);
                    // visual approximation of spacetime curvature (flamm's paraboloid)
                    delta_y = sqrt(8.0 * schwarzschild_radius * (dist - schwarzschild_radius));
                    if (i != 2) // scale down effect for non-black-hole objects
                    {
                        delta_y *= planet_curvature_scale;
                    }
                }
                y += (float)delta_y;
            }
            vertices[vertex_count++] = (vector3_t){world_x, y, world_z};
        }
    }
	physics_unlock();

    for (int z = 0; z < grid_size; ++z)
    {
        for (int x = 0; x < grid_size; ++x)
        {
            int i = z * (grid_size + 1) + x;
            indices[index_count++] = i;
            indices[index_count++] = i + 1;
            indices[index_count++] = i;
            indices[index_count++] = i + grid_size + 1;
        }
    }
    
    if (engine->grid_vao == 0)
    {
        glGenVertexArrays(1, &engine->grid_vao);
        glGenBuffers(1, &engine->grid_vbo);
        glGenBuffers(1, &engine->grid_ebo);
    }

    glBindVertexArray(engine->grid_vao);
    glBindBuffer(GL_ARRAY_BUFFER, engine->grid_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(vector3_t), vertices, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, engine->grid_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(GLuint), indices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vector3_t), (void *)0);
    engine->grid_index_count = index_count;
    glBindVertexArray(0);
    
    free(vertices);
    free(indices);
}

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

