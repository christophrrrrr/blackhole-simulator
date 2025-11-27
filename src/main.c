/**
 * @file main.c
 * @brief this program simulates the gravitational interactions of three celestial bodies (stars and the black hole):
 * two stars and a central supermassive black hole. the rendering is achieved through
 * ray tracing, which calculates the path of light rays as they are bent by the
 * black hole's gravity, according to the principles of general relativity (using
 * the schwarzschild metric).
 *
 * controls:
 * - left mouse + drag: orbit the camera.
 * - middle mouse + drag: pan the camera.
 * - mouse wheel: zoom in/out.
 * - 'r': reset the camera to its initial state.
 * - 'p': pause or resume the physics simulation.
 * - 'g': toggle the visibility of the spacetime grid.
 * - 'esc': exit the application.
 */

#include "math_utils.h"
#include "camera.h"
#include "physics.h"
#include "grid.h"
#include "shaders.h"
#include "renderer.h"
#include "callbacks.h"

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#else
#include <GL/glew.h>
#endif
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
    camera_reset(&camera);

    if (!engine_initialize(&renderer_engine))
    {
        return EXIT_FAILURE;
    }

	physics_start_thread();
	
	// initialize and start grid generation
	grid_init_buffers();
	grid_start_thread();
	grid_update_mesh(&renderer_engine);

    double last_time = glfwGetTime();

    while (!glfwWindowShouldClose(renderer_engine.window))
    {
        double current_time = glfwGetTime();
        double delta_time = current_time - last_time;
        last_time = current_time;

		if (!physics_is_threaded())
		{
			simulation_update_physics(delta_time * 500.0); // speed up simulation time (single-thread fallback)
		}
		
		// update grid mesh from background thread, or generate synchronously if threading not available
		if (grid_is_threaded())
		{
			grid_update_mesh(&renderer_engine);
		}
		else if (!is_physics_paused)
		{
			grid_generate_mesh(&renderer_engine);
		}

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        vector3_t cam_pos = camera_get_position(&camera);
        matrix4_t view_matrix = matrix4_look_at(cam_pos, camera.target, (vector3_t){0, 1, 0});
        matrix4_t projection_matrix = matrix4_perspective(M_PI / 3.0f, (float)renderer_engine.window_width / (float)renderer_engine.window_height, 1e9f, 1e14f);
        matrix4_t view_projection_matrix = matrix4_multiply(projection_matrix, view_matrix);

        grid_render(&renderer_engine, view_projection_matrix);
        engine_render_raytraced_scene_to_texture(&renderer_engine, &camera);
        engine_render_texture_to_screen(&renderer_engine);

        glfwSwapBuffers(renderer_engine.window);
        glfwPollEvents();
    }

	physics_stop_thread();
	grid_stop_thread();
	grid_cleanup_buffers();
    engine_cleanup(&renderer_engine);
    return EXIT_SUCCESS;
}

