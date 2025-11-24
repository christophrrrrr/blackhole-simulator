/**
 * @file math_utils.h
 * @brief vector and matrix math utilities for 3d graphics
 */

#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// structure definitions

/**
 * @struct vector3_t
 * @brief represents a 3d vector.
 */
typedef struct
{
    float x, y, z;
} vector3_t;

/**
 * @struct vector4_t
 * @brief represents a 4d vector.
 */
typedef struct
{
    float x, y, z, w;
} vector4_t;

/**
 * @struct matrix4_t
 * @brief represents a 4x4 matrix in column-major order.
 */
typedef struct
{
    float elements[16];
} matrix4_t;

// utility functions

/**
 * @brief clamps a float value between a minimum and a maximum.
 */
float utility_clamp_float(float value, float min_val, float max_val);

/**
 * @brief calculates the magnitude of a 3d vector.
 */
float vector3_length(vector3_t v);

/**
 * @brief normalizes a 3d vector to unit length.
 */
vector3_t vector3_normalize(vector3_t v);

/**
 * @brief computes the cross product of two 3d vectors.
 */
vector3_t vector3_cross(vector3_t a, vector3_t b);

/**
 * @brief subtracts vector b from vector a.
 */
vector3_t vector3_subtract(vector3_t a, vector3_t b);

/**
 * @brief adds two 3d vectors.
 */
vector3_t vector3_add(vector3_t a, vector3_t b);

/**
 * @brief scales a 3d vector by a scalar value.
 */
vector3_t vector3_scale(vector3_t v, float s);

/**
 * @brief creates a 4x4 identity matrix.
 */
matrix4_t matrix4_identity();

/**
 * @brief creates a perspective projection matrix.
 */
matrix4_t matrix4_perspective(float fovy, float aspect, float z_near, float z_far);

/**
 * @brief creates a view matrix to look at a target from a given position.
 */
matrix4_t matrix4_look_at(vector3_t eye, vector3_t center, vector3_t up);

/**
 * @brief multiplies two 4x4 matrices.
 */
matrix4_t matrix4_multiply(matrix4_t a, matrix4_t b);

#endif // MATH_UTILS_H

