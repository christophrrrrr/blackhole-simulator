/**
 * @file callbacks.h
 * @brief glfw callback functions for input handling
 */

#ifndef CALLBACKS_H
#define CALLBACKS_H

#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif
#include <GLFW/glfw3.h>

/**
 * @brief handles mouse button events
 */
void callback_mouse_button(GLFWwindow *window, int button, int action, int mods);

/**
 * @brief handles cursor position events
 */
void callback_cursor_position(GLFWwindow *window, double xpos, double ypos);

/**
 * @brief handles scroll events
 */
void callback_scroll(GLFWwindow *window, double xoffset, double yoffset);

/**
 * @brief handles keyboard events
 */
void callback_key(GLFWwindow *window, int key, int scancode, int action, int mods);

/**
 * @brief handles framebuffer resize events
 */
void callback_framebuffer_size(GLFWwindow *window, int width, int height);

#endif // CALLBACKS_H

