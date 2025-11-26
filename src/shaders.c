/**
 * @file shaders.c
 * @brief implementation of shader compilation and shader sources
 */

#include "shaders.h"
#include <stdio.h>

GLuint utility_compile_shader(const char *source, GLenum type)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char info_log[512];
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        printf("Shader compilation failed: %s\n", info_log);
        return 0;
    }
    return shader;
}

GLuint utility_create_shader_program(const char *vertex_source, const char *fragment_source)
{
    GLuint vertex_shader = utility_compile_shader(vertex_source, GL_VERTEX_SHADER);
    GLuint fragment_shader = utility_compile_shader(fragment_source, GL_FRAGMENT_SHADER);

    if (!vertex_shader || !fragment_shader) return 0;

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        char info_log[512];
        glGetProgramInfoLog(program, 512, NULL, info_log);
        printf("Shader program linking failed: %s\n", info_log);
        return 0;
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    return program;
}

// shader sources

const char *quad_vertex_shader_source =
    "#version 330 core\n"
    "layout (location = 0) in vec2 aPos;\n"
    "layout (location = 1) in vec2 aTexCoord;\n"
    "out vec2 TexCoord;\n"
    "void main() {\n"
    "    gl_Position = vec4(aPos, 0.0, 1.0);\n"
    "    TexCoord = aTexCoord;\n"
    "}\n";

const char *quad_fragment_shader_source =
    "#version 330 core\n"
    "in vec2 TexCoord;\n"
    "out vec4 FragColor;\n"
    "uniform sampler2D screenTexture;\n"
    "void main() {\n"
    "    FragColor = texture(screenTexture, TexCoord);\n"
    "}\n";

const char *grid_vertex_shader_source =
    "#version 330 core\n"
    "layout(location = 0) in vec3 aPos;\n"
    "uniform mat4 viewProj;\n"
    "void main() {\n"
    "    gl_Position = viewProj * vec4(aPos, 1.0);\n"
    "}\n";

const char *grid_fragment_shader_source =
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main() {\n"
    "    FragColor = vec4(0.5, 0.5, 0.5, 1.0);\n"
    "}\n";

const char *raytracer_fragment_shader_source =
    "#version 330 core\n"
    "in vec2 TexCoord;\n"
    "out vec4 FragColor;\n"
    "\n"
    "// Uniforms\n"
    "uniform vec3 camPos;\n"
    "uniform vec3 camRight;\n"
    "uniform vec3 camUp;\n"
    "uniform vec3 camForward;\n"
    "uniform float tanHalfFov;\n"
    "uniform float aspect;\n"
    "uniform bool moving;\n"
    "uniform float disk_r1;\n"
    "uniform float disk_r2;\n"
    "uniform int numObjects;\n"
    "uniform vec4 objPosRadius[16];\n"
    "uniform vec4 objColor[16];\n"
    "uniform float objMass[16];\n"
    "uniform vec2 resolution;\n"
    "uniform float time;\n"
    "\n"
    "const float blackhole = 1.269e10;\n"
    "float D_LAMBDA = 5e7;\n"
    "const float ESCAPE_R = 1e30;\n"
    "\n"
    "struct Ray {\n"
    "    float x, y, z, r, theta, phi;\n"
    "    float dr, dtheta, dphi;\n"
    "    float E, L;\n"
    "};\n"
    "\n"
    "// Global hit variables\n"
    "vec4 hitObjectColor;\n"
    "vec3 hitCenter;\n"
    "float hitRadius;\n"
    "float random(vec3 p) {\n"
    "    return fract(sin(dot(p, vec3(12.9898, 78.233, 151.7182))) * 43758.5453);\n"
    "}\n"
    "vec4 getStarColor(vec3 dir) {\n"
    "    float star_density = 0.9995;\n"
    "    float r = random(dir);\n"
    "    if (r > star_density) {\n"
    "        float star_brightness = (r - star_density) / (1.0 - star_density);\n"
    "        return vec4(vec3(star_brightness), 1.0);\n"
    "    }\n"
    "    return vec4(0.0);\n"
    "}\n"
    "\n"
    "Ray initRay(vec3 pos, vec3 dir) {\n"
    "    Ray ray;\n"
    "    ray.x = pos.x; ray.y = pos.y; ray.z = pos.z;\n"
    "    ray.r = length(pos);\n"
    "    ray.theta = acos(pos.z / ray.r);\n"
    "    ray.phi = atan(pos.y, pos.x);\n"
    "\n"
    "    float dx = dir.x, dy = dir.y, dz = dir.z;\n"
    "    ray.dr = sin(ray.theta)*cos(ray.phi)*dx + sin(ray.theta)*sin(ray.phi)*dy + cos(ray.theta)*dz;\n"
    "    ray.dtheta = (cos(ray.theta)*cos(ray.phi)*dx + cos(ray.theta)*sin(ray.phi)*dy - sin(ray.theta)*dz) / ray.r;\n"
    "    ray.dphi = (-sin(ray.phi)*dx + cos(ray.phi)*dy) / (ray.r * sin(ray.theta));\n"
    "\n"
    "    ray.L = ray.r * ray.r * sin(ray.theta) * ray.dphi;\n"
    "    float f = 1.0 - blackhole / ray.r;\n"
    "    float dt_dL = sqrt((ray.dr*ray.dr)/f + ray.r*ray.r*(ray.dtheta*ray.dtheta + sin(ray.theta)*sin(ray.theta)*ray.dphi*ray.dphi));\n"
    "    ray.E = f * dt_dL;\n"
    "\n"
    "    return ray;\n"
    "}\n"
    "\n"
    "bool intercept(Ray ray, float rs) {\n"
    "    return ray.r <= rs;\n"
    "}\n"
    "\n"
    "bool interceptObject(Ray ray) {\n"
    "    vec3 P = vec3(ray.x, ray.y, ray.z);\n"
    "    for (int i = 0; i < numObjects; ++i) {\n"
    "        vec3 center = objPosRadius[i].xyz;\n"
    "        float radius = objPosRadius[i].w;\n"
    "        if (distance(P, center) <= radius) {\n"
    "            hitObjectColor = objColor[i];\n"
    "            hitCenter = center;\n"
    "            hitRadius = radius;\n"
    "            return true;\n"
    "        }\n"
    "    }\n"
    "    return false;\n"
    "}\n"
    "\n"
    "void geodesicRHS(Ray ray, out vec3 d1, out vec3 d2) {\n"
    "    float r = ray.r, theta = ray.theta;\n"
    "    float dr = ray.dr, dtheta = ray.dtheta, dphi = ray.dphi;\n"
    "    float f = 1.0 - blackhole / r;\n"
    "    float dt_dL = ray.E / f;\n"
    "\n"
    "    d1 = vec3(dr, dtheta, dphi);\n"
    "    d2.x = -(blackhole / (2.0 * r*r)) * f * dt_dL * dt_dL\n"
    "         + (blackhole / (2.0 * r*r * f)) * dr * dr\n"
    "         + r * (dtheta*dtheta + sin(theta)*sin(theta)*dphi*dphi);\n"
    "    d2.y = -2.0*dr*dtheta/r + sin(theta)*cos(theta)*dphi*dphi;\n"
    "    d2.z = -2.0*dr*dphi/r - 2.0*cos(theta)/(sin(theta)) * dtheta * dphi;\n"
    "}\n"
    "\n"
    "void rk4Step(inout Ray ray, float dL) {\n"
    "    vec3 k1a, k1b;\n"
    "    geodesicRHS(ray, k1a, k1b);\n"
    "    \n"
    "    ray.r      += dL * k1a.x;\n"
    "    ray.theta  += dL * k1a.y;\n"
    "    ray.phi    += dL * k1a.z;\n"
    "    ray.dr     += dL * k1b.x;\n"
    "    ray.dtheta += dL * k1b.y;\n"
    "    ray.dphi   += dL * k1b.z;\n"
    "\n"
    "    ray.x = ray.r * sin(ray.theta) * cos(ray.phi);\n"
    "    ray.y = ray.r * sin(ray.theta) * sin(ray.phi);\n"
    "    ray.z = ray.r * cos(ray.theta);\n"
    "}\n"
    "\n"
    "bool crossesEquatorialPlane(vec3 oldPos, vec3 newPos) {\n"
    "    bool crossed = (oldPos.y * newPos.y < 0.0);\n"
    "    float r = length(vec2(newPos.x, newPos.z));\n"
    "    return crossed && (r >= disk_r1 && r <= disk_r2);\n"
    "}\n"
    "\n"
    "void main() {\n"
    "    vec2 pix = gl_FragCoord.xy;\n"
    "\n"
    "    float u = (2.0 * (pix.x + 0.5) / resolution.x - 1.0) * aspect * tanHalfFov;\n"
    "    float v = (1.0 - 2.0 * (pix.y + 0.5) / resolution.y) * tanHalfFov;\n"
    "    vec3 dir = normalize(u * camRight - v * camUp + camForward);\n"
    "    Ray ray = initRay(camPos, dir);\n"
    "\n"
    "    vec4 color = vec4(0.0);\n"
    "    vec3 prevPos = vec3(ray.x, ray.y, ray.z);\n"
    "\n"
    "    bool hitBlackHole = false;\n"
    "    bool hitDisk = false;\n"
    "    bool hitObject = false;\n"
    "\n"
    "    int steps = moving ? 25000 : 26000;\n"
    "\n"
    "    for (int i = 0; i < steps; ++i) {\n"
    "        if (intercept(ray, blackhole)) { hitBlackHole = true; break; }\n"
    "        float step_scale = clamp(ray.r / (blackhole * 20.0), 0.1, 5.0);\n"
    "        float dynamic_step = D_LAMBDA * step_scale;\n"
    "        rk4Step(ray, dynamic_step);\n"
    "        vec3 newPos = vec3(ray.x, ray.y, ray.z);\n"
    "        if (crossesEquatorialPlane(prevPos, newPos)) { hitDisk = true; break; }\n"
    "        if (interceptObject(ray)) { hitObject = true; break; }\n"
    "        prevPos = newPos;\n"
    "        if (ray.r > ESCAPE_R) break;\n"
    "    }\n"
    "    if (hitDisk) {\n"
    "        vec3 hitPos = vec3(ray.x, ray.y, ray.z);\n"
    "        float r_norm = (length(hitPos) - disk_r1) / (disk_r2 - disk_r1);\n"
    "        r_norm = clamp(r_norm, 0.0, 1.0);\n"
    "        \n"
    "        vec3 color_hot = vec3(1.0, 1.0, 0.8);\n"
    "        vec3 color_mid = vec3(1.0, 0.5, 0.0);\n"
    "        vec3 color_cool = vec3(0.8, 0.0, 0.0);\n"
    "        \n"
    "        vec3 diskColor = mix(color_mid, color_hot, smoothstep(0.0, 0.3, 1.0 - r_norm));\n"
    "        diskColor = mix(color_cool, diskColor, smoothstep(0.3, 1.0, 1.0 - r_norm));\n"
    "        float angle = atan(hitPos.y, hitPos.x);\n"
    "        float spiral = 0.5 + 0.5 * sin(angle * 10.0 - r_norm * 20.0 - time * 0.1);\n"
    "        diskColor *= 0.8 + 0.4 * spiral;\n"
    "        \n"
    "        color = vec4(diskColor, 1.0);\n"
    "    } else if (hitBlackHole) {\n"
    "        color = vec4(0.0, 0.0, 0.0, 1.0);\n"
    "    } else if (hitObject) {\n"
    "        vec3 P = vec3(ray.x, ray.y, ray.z);\n"
    "        vec3 N = normalize(P - hitCenter);\n"
    "        vec3 V = normalize(camPos - P);\n"
    "        vec3 L = normalize(vec3(-1, 1, -1));\n"
    "\n"
    "        float ambient = 0.5;\n"
    "        float diff = max(dot(N, L), 0.0);\n"
    "        vec3 shaded = hitObjectColor.rgb * (ambient + diff);\n"
    "\n"
    "        vec3 H = normalize(L + V);\n"
    "        float spec = pow(max(dot(N, H), 0.0), 32.0);\n"
    "        vec3 specular = vec3(1.0, 1.0, 1.0) * spec * 0.5;\n"
    "\n"
    "        color = vec4(shaded + specular, hitObjectColor.a);\n"
    "    } else {\n"
    "        color = getStarColor(dir);\n"
    "    }\n"
    "\n"
    "    FragColor = color;\n"
    "}\n";

