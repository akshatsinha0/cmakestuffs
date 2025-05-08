/**
 * @file raycaster.h
 * @brief Raycasting engine for 3D rendering
 */
#ifndef RAYCASTER_H
#define RAYCASTER_H

#include "vector.h"
#include "world.h"

 // Ray hit information
typedef struct {
    int hit;             // Whether the ray hit something
    int block_type;      // Type of block hit
    float distance;      // Distance to the hit
    Vector3 position;    // Position of the hit
    Vector3 normal;      // Surface normal at the hit
    int face;            // Face hit (0-5: +x, -x, +y, -y, +z, -z)
    float brightness;    // Lighting at hit point (0.0-1.0)
} RayHit;

// Cast a ray from position in direction
RayHit cast_ray(World* world, Vector3 position, Vector3 direction, float max_distance);

// Get the character to display for a hit
char get_hit_display_char(RayHit hit);

// Get the color code for a hit
int get_hit_color(RayHit hit);

#endif /* RAYCASTER_H */
