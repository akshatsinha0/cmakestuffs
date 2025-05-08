/**
 * @file world.h
 * @brief World representation and block operations
 */
#ifndef WORLD_H
#define WORLD_H

#include <stdint.h>

 // Forward declaration
typedef struct World World;

// Block type definition
typedef struct {
    char display_char;       // Character to display for this block type
    int fg_color;            // Foreground color
    int bg_color;            // Background color
    int solid;               // Whether the block is solid
    float light_absorption;  // How much light this block absorbs
    char* name;              // Name of the block type
} BlockType;

// World structure
struct World {
    int width;               // Width of the world
    int height;              // Height of the world
    int depth;               // Depth of the world
    uint8_t*** blocks;       // 3D array of block types
    BlockType* block_types;  // Array of block type definitions
    int num_block_types;     // Number of block types
    float time_of_day;       // Time of day (0.0-1.0)
    float sky_brightness;    // Sky brightness (0.0-1.0)
};

// World creation and destruction
World* world_create(int width, int height, int depth);
void world_destroy(World* world);

// World generation
void world_generate_terrain(World* world, unsigned int seed);
void world_generate_structures(World* world, unsigned int seed);

// Block operations
uint8_t world_get_block(World* world, int x, int y, int z);
void world_set_block(World* world, int x, int y, int z, uint8_t type);
int world_is_solid(World* world, int x, int y, int z);
int world_is_valid_position(World* world, int x, int y, int z);
float world_get_brightness(World* world, int x, int y, int z);

// Block type operations
void world_init_block_types(World* world);
BlockType world_get_block_type(World* world, uint8_t type);

// World file operations
int world_save(World* world, const char* filename);
World* world_load(const char* filename);

// World lighting
void world_update_lighting(World* world);
void world_set_time(World* world, float time);

#endif /* WORLD_H */
