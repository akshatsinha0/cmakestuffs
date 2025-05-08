/**
* @file raycaster.c
* @brief Implementation of the raycasting engine
*/
#include "raycaster.h"
#include "config.h"
#include "utils.h"
#include <math.h>

// Face normals for each direction
static const Vector3 face_normals[6] = {
    {1.0f, 0.0f, 0.0f},    // +X
    {-1.0f, 0.0f, 0.0f},   // -X
    {0.0f, 1.0f, 0.0f},    // +Y
    {0.0f, -1.0f, 0.0f},   // -Y
    {0.0f, 0.0f, 1.0f},    // +Z
    {0.0f, 0.0f, -1.0f}    // -Z
};

// Face brightness modifiers (simple directional lighting)
static const float face_brightness[6] = {
    0.8f,   // +X
    0.6f,   // -X
    1.0f,   // +Y (top is brightest)
    0.2f,   // -Y (bottom is darkest)
    0.9f,   // +Z
    0.7f    // -Z
};

// Cast a ray and find what it hits
RayHit cast_ray(World* world, Vector3 position, Vector3 direction, float max_distance) {
    RayHit result = { 0 };

    // Initialize ray
    Vector3 ray_pos = position;
    Vector3 ray_dir = vec3_normalize(direction);

    // Initialize result
    result.hit = 0;
    result.distance = max_distance;

    // Ray step and initial step distance
    float t_max_x, t_max_y, t_max_z;
    float t_delta_x, t_delta_y, t_delta_z;
    int step_x, step_y, step_z;
    int map_x, map_y, map_z;

    // Current map cell coordinates
    map_x = (int)floorf(ray_pos.x);
    map_y = (int)floorf(ray_pos.y);
    map_z = (int)floorf(ray_pos.z);

    // Direction step and initial step distance
    if (ray_dir.x == 0.0f) {
        t_delta_x = 1e30f;
        t_max_x = 1e30f;
        step_x = 0;
    }
    else if (ray_dir.x > 0.0f) {
        step_x = 1;
        t_delta_x = 1.0f / ray_dir.x;
        t_max_x = t_delta_x * ((float)(map_x + 1) - ray_pos.x);
    }
    else {
        step_x = -1;
        t_delta_x = 1.0f / -ray_dir.x;
        t_max_x = t_delta_x * (ray_pos.x - (float)map_x);
    }

    if (ray_dir.y == 0.0f) {
        t_delta_y = 1e30f;
        t_max_y = 1e30f;
        step_y = 0;
    }
    else if (ray_dir.y > 0.0f) {
        step_y = 1;
        t_delta_y = 1.0f / ray_dir.y;
        t_max_y = t_delta_y * ((float)(map_y + 1) - ray_pos.y);
    }
    else {
        step_y = -1;
        t_delta_y = 1.0f / -ray_dir.y;
        t_max_y = t_delta_y * (ray_pos.y - (float)map_y);
    }

    if (ray_dir.z == 0.0f) {
        t_delta_z = 1e30f;
        t_max_z = 1e30f;
        step_z = 0;
    }
    else if (ray_dir.z > 0.0f) {
        step_z = 1;
        t_delta_z = 1.0f / ray_dir.z;
        t_max_z = t_delta_z * ((float)(map_z + 1) - ray_pos.z);
    }
    else {
        step_z = -1;
        t_delta_z = 1.0f / -ray_dir.z;
        t_max_z = t_delta_z * (ray_pos.z - (float)map_z);
    }

    // DDA algorithm for raycasting
    float distance = 0.0f;
    int face = 0;

    while (distance < max_distance) {
        // Find the closest axis to step along
        if (t_max_x < t_max_y && t_max_x < t_max_z) {
            distance = t_max_x;
            t_max_x += t_delta_x;
            map_x += step_x;
            face = (step_x > 0) ? 1 : 0; // -X or +X face
        }
        else if (t_max_y < t_max_z) {
            distance = t_max_y;
            t_max_y += t_delta_y;
            map_y += step_y;
            face = (step_y > 0) ? 3 : 2; // -Y or +Y face
        }
        else {
            distance = t_max_z;
            t_max_z += t_delta_z;
            map_z += step_z;
            face = (step_z > 0) ? 5 : 4; // -Z or +Z face
        }

        // Check bounds
        if (map_x < 0 || map_y < 0 || map_z < 0 ||
            map_x >= world->width || map_y >= world->height || map_z >= world->depth) {
            break; // Out of bounds
        }

        // Check hit
        uint8_t block_type = world_get_block(world, map_x, map_y, map_z);
        if (block_type != BLOCK_AIR) {
            // We hit something!
            result.hit = 1;
            result.block_type = block_type;
            result.distance = distance;
            result.position = vec3_add(ray_pos, vec3_mul(ray_dir, distance));
            result.normal = face_normals[face];
            result.face = face;

            // Calculate lighting
            float brightness = world_get_brightness(world, map_x, map_y, map_z);
            brightness *= face_brightness[face];

            // Apply fog based on distance
            if (ENABLE_FOG) {
                float fog_factor = clamp((distance - FOG_START) / (FOG_END - FOG_START), 0.0f, 1.0f);
                brightness *= (1.0f - fog_factor * 0.8f);
            }

            result.brightness = clamp(brightness, 0.2f, 1.0f);
            break;
        }
    }

    return result;
}

// Get character to display for a hit
char get_hit_display_char(RayHit hit) {
    if (!hit.hit) {
        return ' ';
    }

    // Get block type display character
    BlockType block_type = world_get_block_type(NULL, hit.block_type);

    // Edge detection for better visual definition
    Vector3 local_pos = hit.position;
    local_pos.x -= floorf(local_pos.x);
    local_pos.y -= floorf(local_pos.y);
    local_pos.z -= floorf(local_pos.z);

    // Detect edges
    int on_edge = 0;
    if (hit.face == 0 || hit.face == 1) { // X faces
        on_edge = (local_pos.y < EDGE_THRESHOLD || local_pos.y > 1.0f - EDGE_THRESHOLD ||
            local_pos.z < EDGE_THRESHOLD || local_pos.z > 1.0f - EDGE_THRESHOLD);
    }
    else if (hit.face == 2 || hit.face == 3) { // Y faces
        on_edge = (local_pos.x < EDGE_THRESHOLD || local_pos.x > 1.0f - EDGE_THRESHOLD ||
            local_pos.z < EDGE_THRESHOLD || local_pos.z > 1.0f - EDGE_THRESHOLD);
    }
    else { // Z faces
        on_edge = (local_pos.x < EDGE_THRESHOLD || local_pos.x > 1.0f - EDGE_THRESHOLD ||
            local_pos.y < EDGE_THRESHOLD || local_pos.y > 1.0f - EDGE_THRESHOLD);
    }

    // Return either block character or edge character
    return on_edge ? '#' : block_type.display_char;
}

// Get color for a hit
int get_hit_color(RayHit hit) {
    if (!hit.hit) {
        return COLOR_BLACK;
    }

    // Get block type
    BlockType block_type = world_get_block_type(NULL, hit.block_type);

    // Determine color based on brightness
    int color = block_type.fg_color;

    // Adjust color based on brightness if shading is enabled
    if (ENABLE_SHADING) {
        if (hit.brightness < 0.4f) {
            color &= ~COLOR_BRIGHT; // Remove brightness
        }
        else if (hit.brightness > 0.8f) {
            color |= COLOR_BRIGHT;  // Add brightness
        }
    }

    return color;
}
