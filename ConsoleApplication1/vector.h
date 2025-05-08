/**
 * @file vector.h
 * @brief Vector math operations for 2D and 3D vectors
 */
#ifndef VECTOR_H
#define VECTOR_H

 // 2D vector structure
typedef struct {
    float x;
    float y;
} Vector2;

// 3D vector structure
typedef struct {
    float x;
    float y;
    float z;
} Vector3;

// Vector2 operations
Vector2 vec2_create(float x, float y);
Vector2 vec2_add(Vector2 a, Vector2 b);
Vector2 vec2_sub(Vector2 a, Vector2 b);
Vector2 vec2_mul(Vector2 a, float scalar);
Vector2 vec2_div(Vector2 a, float scalar);
float vec2_length(Vector2 v);
float vec2_dot(Vector2 a, Vector2 b);
Vector2 vec2_normalize(Vector2 v);
Vector2 vec2_rotate(Vector2 v, float angle);

// Vector3 operations
Vector3 vec3_create(float x, float y, float z);
Vector3 vec3_add(Vector3 a, Vector3 b);
Vector3 vec3_sub(Vector3 a, Vector3 b);
Vector3 vec3_mul(Vector3 a, float scalar);
Vector3 vec3_div(Vector3 a, float scalar);
float vec3_length(Vector3 v);
float vec3_dot(Vector3 a, Vector3 b);
Vector3 vec3_cross(Vector3 a, Vector3 b);
Vector3 vec3_normalize(Vector3 v);
Vector3 vec3_from_angles(float pitch, float yaw);
Vector3 vec3_lerp(Vector3 a, Vector3 b, float t);

#endif /* VECTOR_H */
