/**
 * @file vector.c
 * @brief Implementation of vector operations
 */
#include "vector.h"
#include <math.h>

 // Vector2 operations
Vector2 vec2_create(float x, float y) {
    Vector2 v = { x, y };
    return v;
}

Vector2 vec2_add(Vector2 a, Vector2 b) {
    return vec2_create(a.x + b.x, a.y + b.y);
}

Vector2 vec2_sub(Vector2 a, Vector2 b) {
    return vec2_create(a.x - b.x, a.y - b.y);
}

Vector2 vec2_mul(Vector2 a, float scalar) {
    return vec2_create(a.x * scalar, a.y * scalar);
}

Vector2 vec2_div(Vector2 a, float scalar) {
    if (scalar == 0.0f) {
        return a; // Prevent division by zero
    }
    return vec2_create(a.x / scalar, a.y / scalar);
}

float vec2_length(Vector2 v) {
    return sqrtf(v.x * v.x + v.y * v.y);
}

float vec2_dot(Vector2 a, Vector2 b) {
    return a.x * b.x + a.y * b.y;
}

Vector2 vec2_normalize(Vector2 v) {
    float length = vec2_length(v);
    if (length < 0.0001f) {
        return vec2_create(0.0f, 0.0f);
    }
    return vec2_div(v, length);
}

Vector2 vec2_rotate(Vector2 v, float angle) {
    float cs = cosf(angle);
    float sn = sinf(angle);
    return vec2_create(
        v.x * cs - v.y * sn,
        v.x * sn + v.y * cs
    );
}

// Vector3 operations
Vector3 vec3_create(float x, float y, float z) {
    Vector3 v = { x, y, z };
    return v;
}

Vector3 vec3_add(Vector3 a, Vector3 b) {
    return vec3_create(a.x + b.x, a.y + b.y, a.z + b.z);
}

Vector3 vec3_sub(Vector3 a, Vector3 b) {
    return vec3_create(a.x - b.x, a.y - b.y, a.z - b.z);
}

Vector3 vec3_mul(Vector3 a, float scalar) {
    return vec3_create(a.x * scalar, a.y * scalar, a.z * scalar);
}

Vector3 vec3_div(Vector3 a, float scalar) {
    if (scalar == 0.0f) {
        return a; // Prevent division by zero
    }
    return vec3_create(a.x / scalar, a.y / scalar, a.z / scalar);
}

float vec3_length(Vector3 v) {
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

float vec3_dot(Vector3 a, Vector3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vector3 vec3_cross(Vector3 a, Vector3 b) {
    return vec3_create(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

Vector3 vec3_normalize(Vector3 v) {
    float length = vec3_length(v);
    if (length < 0.0001f) {
        return vec3_create(0.0f, 0.0f, 0.0f);
    }
    return vec3_div(v, length);
}

Vector3 vec3_from_angles(float pitch, float yaw) {
    return vec3_create(
        cosf(pitch) * cosf(yaw),
        cosf(pitch) * sinf(yaw),
        sinf(pitch)
    );
}

Vector3 vec3_lerp(Vector3 a, Vector3 b, float t) {
    if (t <= 0.0f) return a;
    if (t >= 1.0f) return b;
    return vec3_create(
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
        a.z + (b.z - a.z) * t
    );
}
