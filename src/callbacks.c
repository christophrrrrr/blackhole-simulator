/**
 * @file callbacks.c
 * @brief implementation of glfw callbacks
 */

#include "callbacks.h"
#include "camera.h"
#include "physics.h"
#include "grid.h"
#include "renderer.h"
#include <stdio.h>

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#else
#include <GL/glew.h>
#endif

void callback_mouse_button(GLFWwindow *window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            camera.is_dragging_orbit = true;
            glfwGetCursorPos(window, &camera.last_cursor_x, &camera.last_cursor_y);
        }
        else if (action == GLFW_RELEASE)
        {
            camera.is_dragging_orbit = false;
        }
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE)
    {
        if (action == GLFW_PRESS)
        {
            camera.is_dragging_pan = true;
            glfwGetCursorPos(window, &camera.last_cursor_x, &camera.last_cursor_y);
        }
        else if (action == GLFW_RELEASE)
        {
            camera.is_dragging_pan = false;
        }
    }
    camera_update_moving_state(&camera);
}

void callback_cursor_position(GLFWwindow *window, double xpos, double ypos)
{
    camera_process_mouse_move(&camera, xpos, ypos);
}

void callback_scroll(GLFWwindow *window, double xoffset, double yoffset)
{
    camera_process_scroll(&camera, yoffset);
}

void callback_key(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        switch (key)
        {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        case GLFW_KEY_R:
            camera_reset(&camera);
            printf("[INFO] Camera reset\n");
            break;
        case GLFW_KEY_P:
            is_physics_paused = !is_physics_paused;
            printf("[INFO] Physics %s\n", is_physics_paused ? "paused" : "resumed");
            break;
        case GLFW_KEY_G:
            is_grid_visible = !is_grid_visible;
            printf("[INFO] Grid %s\n", is_grid_visible ? "visible" : "hidden");
            break;
        }
    }
}

void callback_framebuffer_size(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
    renderer_engine.window_width = width;
    renderer_engine.window_height = height;

    renderer_engine.render_texture_width = width;
    renderer_engine.render_texture_height = height;
    glBindTexture(GL_TEXTURE_2D, renderer_engine.render_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, renderer_engine.render_texture_width, renderer_engine.render_texture_height,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);
}

