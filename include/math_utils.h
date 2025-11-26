#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif



// structure definitions
typedef struct
{
    float x, y, z;
} vector3_t;

typedef struct
{
    float x, y, z, w;
} vector4_t;

typedef struct
{
    float elements[16];
} matrix4_t;



// utility functions

float utility_clamp_float(float value, float min_val, float max_val);

float vector3_length(vector3_t v);

vector3_t vector3_normalize(vector3_t v);

vector3_t vector3_cross(vector3_t a, vector3_t b);

vector3_t vector3_subtract(vector3_t a, vector3_t b);

vector3_t vector3_add(vector3_t a, vector3_t b);

vector3_t vector3_scale(vector3_t v, float s);

matrix4_t matrix4_identity();

matrix4_t matrix4_perspective(float fovy, float aspect, float z_near, float z_far);

matrix4_t matrix4_look_at(vector3_t eye, vector3_t center, vector3_t up);

matrix4_t matrix4_multiply(matrix4_t a, matrix4_t b);

#endif // MATH_UTILS_H

