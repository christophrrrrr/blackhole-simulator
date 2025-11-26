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

// renderer engine
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

// initializes glfw, opengl context, shaders, and all rendering objects.
bool engine_initialize(renderer_engine_t *engine);

// initializes a vertex array object for drawing a fullscreen quad.
void engine_init_fullscreen_quad(renderer_engine_t *engine);

// initializes the texture used as a render target for the ray tracer.
void engine_init_render_texture(renderer_engine_t *engine);

// renders the main scene using the ray tracing shader into a texture.
void engine_render_raytraced_scene_to_texture(renderer_engine_t *engine, camera_t *cam);

// renders the previously generated texture to the screen.
void engine_render_texture_to_screen(renderer_engine_t *engine);

// cleans up all allocated resources.
void engine_cleanup(renderer_engine_t *engine);

#endif // RENDERER_H

