#ifndef SHADERS_H
#define SHADERS_H

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#else
#include <GL/glew.h>
#endif

// compiles a shader from a source string
GLuint utility_compile_shader(const char *source, GLenum type);

// creates a shader program by linking vertex and fragment shaders
GLuint utility_create_shader_program(const char *vertex_source, const char *fragment_source);

// shader source code
extern const char *quad_vertex_shader_source;
extern const char *quad_fragment_shader_source;
extern const char *grid_vertex_shader_source;
extern const char *grid_fragment_shader_source;
extern const char *raytracer_fragment_shader_source;

#endif // SHADERS_H

