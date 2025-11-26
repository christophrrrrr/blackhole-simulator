#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <GLFW/glfw3.h>

void callback_mouse_button(GLFWwindow *window, int button, int action, int mods);

void callback_cursor_position(GLFWwindow *window, double xpos, double ypos);

void callback_scroll(GLFWwindow *window, double xoffset, double yoffset);

void callback_key(GLFWwindow *window, int key, int scancode, int action, int mods);

void callback_framebuffer_size(GLFWwindow *window, int width, int height);

#endif // CALLBACKS_H

