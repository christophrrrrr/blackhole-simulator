/**
 * @file renderer.h
 * @brief opengl rendering engine
 */

#ifndef RENDERER_H
#define RENDERER_H

#include "math_utils.h"
#include "camera.h"

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#else
#include <GL/glew.h>
#endif
#include <GLFW/glfw3.h>
#include <stdbool.h>

/**
 * @struct renderer_engine_t
 * @brief encapsulates all opengl and glfw objects required for rendering.
 */
typedef struct
{
    GLFWwindow *window;
    GLuint fullscreen_quad_vao;
    GLuint render_texture;
    GLuint raytracer_shader_program;
    GLuint grid_shader_program;
    GLuint texture_quad_shader_program;
    GLuint grid_vao, grid_vbo, grid_ebo;
    int grid_index_count;
    int window_width, window_height;
    int render_texture_width, render_texture_height;
} renderer_engine_t;

// global renderer engine
extern renderer_engine_t renderer_engine;

/**
 * @brief initializes glfw, opengl context, shaders, and all rendering objects.
 * @return true on success, false on failure.
 */
bool engine_initialize(renderer_engine_t *engine);

/**
 * @brief initializes a vertex array object for drawing a fullscreen quad.
 */
void engine_init_fullscreen_quad(renderer_engine_t *engine);

/**
 * @brief initializes the texture used as a render target for the ray tracer.
 */
void engine_init_render_texture(renderer_engine_t *engine);

/**
 * @brief renders the main scene using the ray tracing shader into a texture.
 */
void engine_render_raytraced_scene_to_texture(renderer_engine_t *engine, camera_t *cam);

/**
 * @brief renders the previously generated texture to the screen.
 */
void engine_render_texture_to_screen(renderer_engine_t *engine);

/**
 * @brief cleans up all allocated resources.
 */
void engine_cleanup(renderer_engine_t *engine);

#endif // RENDERER_H

