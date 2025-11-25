/**
 * @file renderer.c
 * @brief implementation of the opengl rendering engine
 */

#include "renderer.h"
#include "shaders.h"
#include "physics.h"
#include "callbacks.h"
#include <stdio.h>
#include <math.h>

renderer_engine_t renderer_engine;

void engine_init_fullscreen_quad(renderer_engine_t *engine)
{
    float quad_vertices[] = {
        -1.0f, 1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f};

    GLuint vbo;
    glGenVertexArrays(1, &engine->fullscreen_quad_vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(engine->fullscreen_quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void engine_init_render_texture(renderer_engine_t *engine)
{
    glGenTextures(1, &engine->render_texture);
    glBindTexture(GL_TEXTURE_2D, engine->render_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, engine->render_texture_width, engine->render_texture_height,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void engine_render_raytraced_scene_to_texture(renderer_engine_t *engine, camera_t *cam)
{
    GLuint framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, engine->render_texture, 0);

    glViewport(0, 0, engine->render_texture_width, engine->render_texture_height);
    glUseProgram(engine->raytracer_shader_program);

    vector3_t pos = camera_get_position(cam);
    vector3_t fwd = vector3_normalize(vector3_subtract(cam->target, pos));
    vector3_t global_up = {0, 1, 0};
    vector3_t right = vector3_normalize(vector3_cross(fwd, global_up));
    vector3_t up = vector3_cross(right, fwd);

    glUniform3f(glGetUniformLocation(engine->raytracer_shader_program, "camPos"), pos.x, pos.y, pos.z);
    glUniform3f(glGetUniformLocation(engine->raytracer_shader_program, "camRight"), right.x, right.y, right.z);
    glUniform3f(glGetUniformLocation(engine->raytracer_shader_program, "camUp"), up.x, up.y, up.z);
    glUniform3f(glGetUniformLocation(engine->raytracer_shader_program, "camForward"), fwd.x, fwd.y, fwd.z);
    glUniform1f(glGetUniformLocation(engine->raytracer_shader_program, "tanHalfFov"), tanf(M_PI / 6.0f));
    glUniform1f(glGetUniformLocation(engine->raytracer_shader_program, "aspect"), (float)engine->window_width / (float)engine->window_height);
    glUniform1i(glGetUniformLocation(engine->raytracer_shader_program, "moving"), cam->is_moving ? 1 : 0);
    glUniform2f(glGetUniformLocation(engine->raytracer_shader_program, "resolution"),
                (float)engine->render_texture_width, (float)engine->render_texture_height);
    glUniform1f(glGetUniformLocation(engine->raytracer_shader_program, "time"), (float)glfwGetTime());

    float disk_inner_radius = BLACK_HOLE_SCHWARZSCHILD_RADIUS * 2.2f;
    float disk_outer_radius = BLACK_HOLE_SCHWARZSCHILD_RADIUS * 5.2f;
    glUniform1f(glGetUniformLocation(engine->raytracer_shader_program, "disk_r1"), disk_inner_radius);
    glUniform1f(glGetUniformLocation(engine->raytracer_shader_program, "disk_r2"), disk_outer_radius);

    glUniform1i(glGetUniformLocation(engine->raytracer_shader_program, "numObjects"), NUM_CELESTIAL_BODIES);
    for (int i = 0; i < NUM_CELESTIAL_BODIES; ++i)
    {
        char uniform_name[64];
        snprintf(uniform_name, sizeof(uniform_name), "objPosRadius[%d]", i);
        glUniform4fv(glGetUniformLocation(engine->raytracer_shader_program, uniform_name), 1, &celestial_bodies[i].position_and_radius.x);
        snprintf(uniform_name, sizeof(uniform_name), "objColor[%d]", i);
        glUniform4fv(glGetUniformLocation(engine->raytracer_shader_program, uniform_name), 1, &celestial_bodies[i].color.x);
        snprintf(uniform_name, sizeof(uniform_name), "objMass[%d]", i);
        glUniform1f(glGetUniformLocation(engine->raytracer_shader_program, uniform_name), celestial_bodies[i].mass);
    }
    
    glBindVertexArray(engine->fullscreen_quad_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &framebuffer);
}

void engine_render_texture_to_screen(renderer_engine_t *engine)
{
    glViewport(0, 0, engine->window_width, engine->window_height);

    glUseProgram(engine->texture_quad_shader_program);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, engine->render_texture);
    glUniform1i(glGetUniformLocation(engine->texture_quad_shader_program, "screenTexture"), 0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    
    glBindVertexArray(engine->fullscreen_quad_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glBindVertexArray(0);
}

bool engine_initialize(renderer_engine_t *engine)
{
    if (!glfwInit())
    {
        printf("Failed to initialize GLFW\n");
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    engine->window = glfwCreateWindow(500, 300, "Black Hole", NULL, NULL);
    if (!engine->window)
    {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(engine->window);
#ifndef __APPLE__
    GLenum glew_err = glewInit();
    if (glew_err != GLEW_OK)
    {
        printf("Failed to initialize GLEW: %s\n", glewGetErrorString(glew_err));
        return false;
    }
#endif
    glfwSetFramebufferSizeCallback(engine->window, callback_framebuffer_size);
    glfwGetFramebufferSize(engine->window, &engine->window_width, &engine->window_height);
    glViewport(0, 0, engine->window_width, engine->window_height);

    engine->render_texture_width = engine->window_width / 7; // low resolution to improve performance
    engine->render_texture_height = engine->window_height / 7;

    glfwSwapInterval(1); // v-sync

    printf("--- Black Hole ---\n");
    printf("Initial Framebuffer Size: %d x %d pixels\n", engine->window_width, engine->window_height);
    printf("Compute Resolution: %d x %d pixels\n", engine->render_texture_width, engine->render_texture_height);
    printf("--- CONTROLS ---\n");
    printf("Left Mouse + Drag: Orbit Camera\n");
    printf("Middle Mouse + Drag: Pan Camera\n");
    printf("Mouse Wheel: Zoom\n");
    printf("R: Reset Camera\n");
    printf("P: Pause/Resume Physics\n");
    printf("G: Toggle Spacetime Grid\n");
    printf("ESC: Exit\n");
    printf("----------------\n");

    engine->raytracer_shader_program = utility_create_shader_program(quad_vertex_shader_source, raytracer_fragment_shader_source);
    engine->grid_shader_program = utility_create_shader_program(grid_vertex_shader_source, grid_fragment_shader_source);
    engine->texture_quad_shader_program = utility_create_shader_program(quad_vertex_shader_source, quad_fragment_shader_source);

    if (!engine->raytracer_shader_program || !engine->grid_shader_program || !engine->texture_quad_shader_program)
    {
        return false;
    }

    engine_init_fullscreen_quad(engine);
    engine_init_render_texture(engine);

    glfwSetMouseButtonCallback(engine->window, callback_mouse_button);
    glfwSetCursorPosCallback(engine->window, callback_cursor_position);
    glfwSetScrollCallback(engine->window, callback_scroll);
    glfwSetKeyCallback(engine->window, callback_key);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    return true;
}

void engine_cleanup(renderer_engine_t *engine)
{
    if (engine->fullscreen_quad_vao) glDeleteVertexArrays(1, &engine->fullscreen_quad_vao);
    if (engine->render_texture) glDeleteTextures(1, &engine->render_texture);
    if (engine->raytracer_shader_program) glDeleteProgram(engine->raytracer_shader_program);
    if (engine->grid_shader_program) glDeleteProgram(engine->grid_shader_program);
    if (engine->texture_quad_shader_program) glDeleteProgram(engine->texture_quad_shader_program);
    if (engine->grid_vao) glDeleteVertexArrays(1, &engine->grid_vao);
    if (engine->grid_vbo) glDeleteBuffers(1, &engine->grid_vbo);
    if (engine->grid_ebo) glDeleteBuffers(1, &engine->grid_ebo);

    if (engine->window)
    {
        glfwDestroyWindow(engine->window);
    }
    glfwTerminate();
}

