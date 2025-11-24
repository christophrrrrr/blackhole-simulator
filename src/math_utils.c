/**
 * @file math_utils.c
 * @brief implementation of vector and matrix math utilities
 */

#include "math_utils.h"
#include <math.h>

float utility_clamp_float(float value, float min_val, float max_val)
{
    if (value < min_val) return min_val;
    if (value > max_val) return max_val;
    return value;
}

float vector3_length(vector3_t v)
{
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

vector3_t vector3_normalize(vector3_t v)
{
    float len = vector3_length(v);
    if (len > 0.0f)
    {
        return (vector3_t){v.x / len, v.y / len, v.z / len};
    }
    return (vector3_t){0.0f, 0.0f, 0.0f};
}

vector3_t vector3_cross(vector3_t a, vector3_t b)
{
    return (vector3_t){
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x};
}

vector3_t vector3_subtract(vector3_t a, vector3_t b)
{
    return (vector3_t){a.x - b.x, a.y - b.y, a.z - b.z};
}

vector3_t vector3_add(vector3_t a, vector3_t b)
{
    return (vector3_t){a.x + b.x, a.y + b.y, a.z + b.z};
}

vector3_t vector3_scale(vector3_t v, float s)
{
    return (vector3_t){v.x * s, v.y * s, v.z * s};
}

matrix4_t matrix4_identity()
{
    matrix4_t m = {0};
    m.elements[0] = m.elements[5] = m.elements[10] = m.elements[15] = 1.0f;
    return m;
}

matrix4_t matrix4_perspective(float fovy, float aspect, float z_near, float z_far)
{
    matrix4_t result = {0};
    float tan_half_fovy = tanf(fovy / 2.0f);
    result.elements[0] = 1.0f / (aspect * tan_half_fovy);
    result.elements[5] = 1.0f / tan_half_fovy;
    result.elements[10] = -(z_far + z_near) / (z_far - z_near);
    result.elements[11] = -1.0f;
    result.elements[14] = -(2.0f * z_far * z_near) / (z_far - z_near);
    return result;
}

matrix4_t matrix4_look_at(vector3_t eye, vector3_t center, vector3_t up)
{
    vector3_t f = vector3_normalize(vector3_subtract(center, eye));
    vector3_t s = vector3_normalize(vector3_cross(f, up));
    vector3_t u = vector3_cross(s, f);
    matrix4_t result = matrix4_identity();
    result.elements[0] = s.x;
    result.elements[4] = s.y;
    result.elements[8] = s.z;
    result.elements[1] = u.x;
    result.elements[5] = u.y;
    result.elements[9] = u.z;
    result.elements[2] = -f.x;
    result.elements[6] = -f.y;
    result.elements[10] = -f.z;
    result.elements[12] = -(s.x * eye.x + s.y * eye.y + s.z * eye.z);
    result.elements[13] = -(u.x * eye.x + u.y * eye.y + u.z * eye.z);
    result.elements[14] = f.x * eye.x + f.y * eye.y + f.z * eye.z;
    return result;
}

matrix4_t matrix4_multiply(matrix4_t a, matrix4_t b)
{
    matrix4_t result = {0};
    for (int c = 0; c < 4; ++c)
    {
        for (int r = 0; r < 4; ++r)
        {
            float sum = 0.0f;
            for (int k = 0; k < 4; ++k)
            {
                sum += a.elements[k * 4 + r] * b.elements[c * 4 + k];
            }
            result.elements[c * 4 + r] = sum;
        }
    }
    return result;
}

