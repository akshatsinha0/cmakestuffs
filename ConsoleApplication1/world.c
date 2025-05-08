/**
 * @file world.c
 * @brief Implementation of world generation and management
 */
#include "world.h"
#include "config.h"
#include "utils.h"
#include "terminal.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

 // Global block type definitions
static BlockType default_block_types[MAX_BLOCK_TYPES] = {
    // Air
    { ' ', COLOR_BLACK, COLOR_BLACK, 0, 0.0f, "Air" },
    // Dirt
    { '.', COLOR_YELLOW, COLOR_BLACK, 1, 0.6f, "Dirt" },
    // Grass
    { '"', COLOR_GREEN, COLOR_BLACK, 1, 0.5f, "Grass" },
    // Stone
    { '#', COLOR_WHITE, COLOR_BLACK, 1, 0.8f, "Stone" },
    // Wood
    { '|', COLOR_YELLOW | COLOR_BRIGHT, COLOR_BLACK, 1, 0.7f, "Wood" },
    // Leaves
    { '*', COLOR_GREEN | COLOR_BRIGHT, COLOR_BLACK, 1, 0.5f, "Leaves" },
    // Water
    { '~', COLOR_BLUE, COLOR_BLACK, 0, 0.3f, "Water" },
    // Sand
    { ',', COLOR_YELLOW | COLOR_BRIGHT, COLOR_BLACK, 1, 0.4f, "Sand" },
    // Brick
    { '=', COLOR_RED, COLOR_BLACK, 1, 0.9f, "Brick" }
};

// Create a new world
World* world_create(int width, int height, int depth) {
    World* world = (World*)malloc(sizeof(World));
    if (!world) return NULL;

    // Set dimensions
    world->width = width;
    world->height = height;
    world->depth = depth;

    // Allocate blocks
    world->blocks = (uint8_t***)malloc(depth * sizeof(uint8_t**));
    if (!world->blocks) {
        free(world);
        return NULL;
    }

    for (int z = 0; z < depth; z++) {
        world->blocks[z] = (uint8_t**)malloc(height * sizeof(uint8_t*));
        if (!world->blocks[z]) {
            // Clean up
            for (int i = 0; i < z; i++) {
                for (int j = 0; j < height; j++) {
                    free(world->blocks[i][j]);
                }
                free(world->blocks[i]);
            }
            free(world->blocks);
            free(world);
            return NULL;
        }

        for (int y = 0; y < height; y++) {
            world->blocks[z][y] = (uint8_t*)malloc(width * sizeof(uint8_t));
            if (!world->blocks[z][y]) {
                // Clean up
                for (int i = 0; i < z; i++) {
                    for (int j = 0; j < height; j++) {
                        free(world->blocks[i][j]);
                    }
                    free(world->blocks[i]);
                }

                for (int j = 0; j < y; j++) {
                    free(world->blocks[z][j]);
                }
                free(world->blocks[z]);
                free(world->blocks);
                free(world);
                return NULL;
            }

            // Initialize all blocks to air
            memset(world->blocks[z][y], BLOCK_AIR, width * sizeof(uint8_t));
        }
    }

    // Initialize block types
    world->block_types = (BlockType*)malloc(MAX_BLOCK_TYPES * sizeof(BlockType));
    if (!world->block_types) {
        world_destroy(world);
        return NULL;
    }

    // Copy default block types
    memcpy(world->block_types, default_block_types, MAX_BLOCK_TYPES * sizeof(BlockType));
    world->num_block_types = MAX_BLOCK_TYPES;

    // Initialize time of day and sky brightness
    world->time_of_day = 0.5f;
    world->sky_brightness = 1.0f;

    return world;
}

// Destroy a world
void world_destroy(World* world) {
    if (!world) return;

    // Free blocks
    if (world->blocks) {
        for (int z = 0; z < world->depth; z++) {
            if (world->blocks[z]) {
                for (int y = 0; y < world->height; y++) {
                    free(world->blocks[z][y]);
                }
                free(world->blocks[z]);
            }
        }
        free(world->blocks);
    }

    // Free block types
    free(world->block_types);

    // Free world
    free(world);
}

// Generate terrain using perlin noise
void world_generate_terrain(World* world, unsigned int seed) {
    if (!world) return;

    // Set random seed
    set_random_seed(seed);

    // Generate heightmap
    int* heightmap = (int*)malloc(world->width * world->height * sizeof(int));
    if (!heightmap) return;

    // Terrain parameters
    float scale_x = 0.05f;
    float scale_y = 0.05f;
    int octaves = 4;
    float persistence = 0.5f;

    // Generate heightmap
    for (int y = 0; y < world->height; y++) {
        for (int x = 0; x < world->width; x++) {
            float nx = x * scale_x;
            float ny = y * scale_y;

            // Generate base terrain
            float height = perlin_noise2d(nx, ny, octaves, persistence, seed);

            // Normalize to world depth
            height = (height * 0.5f + 0.5f) * (world->depth * 0.7f);
            heightmap[y * world->width + x] = (int)height;

            // Ensure minimum ground height
            if (heightmap[y * world->width + x] < GROUND_HEIGHT) {
                heightmap[y * world->width + x] = GROUND_HEIGHT;
            }
        }
    }

    // Fill terrain
    for (int y = 0; y < world->height; y++) {
        for (int x = 0; x < world->width; x++) {
            int surface_height = heightmap[y * world->width + x];

            for (int z = 0; z < world->depth; z++) {
                if (z < surface_height - 4) {
                    // Deep stone
                    world->blocks[z][y][x] = BLOCK_STONE;
                }
                else if (z < surface_height - 1) {
                    // Dirt
                    world->blocks[z][y][x] = BLOCK_DIRT;
                }
                else if (z == surface_height - 1) {
                    // Surface layer
                    float moisture = perlin_noise2d(x * 0.1f, y * 0.1f, 2, 0.5f, seed + 1);
                    if (moisture > 0.6f) {
                        // Water area
                        world->blocks[z][y][x] = BLOCK_DIRT;
                        if (z + 1 < world->depth) {
                            world->blocks[z + 1][y][x] = BLOCK_WATER;
                        }
                    }
                    else if (moisture < -0.3f) {
                        // Sandy area
                        world->blocks[z][y][x] = BLOCK_SAND;
                    }
                    else {
                        // Grass
                        world->blocks[z][y][x] = BLOCK_GRASS;
                    }
                }
            }
        }
    }

    // Generate trees
    int tree_count = (world->width * world->height) / 100;
    for (int i = 0; i < tree_count; i++) {
        int tx = random_int(3, world->width - 4);
        int ty = random_int(3, world->height - 4);
        int surface_height = heightmap[ty * world->width + tx];

        // Only place trees on grass
        if (surface_height < world->depth - 5 &&
            world->blocks[surface_height - 1][ty][tx] == BLOCK_GRASS) {

            // Tree height
            int tree_height = random_int(4, 7);

            // Tree trunk
            for (int tz = surface_height; tz < surface_height + tree_height; tz++) {
                if (tz < world->depth) {
                    world->blocks[tz][ty][tx] = BLOCK_WOOD;
                }
            }

            // Tree leaves
            for (int lz = surface_height + tree_height - 3; lz < surface_height + tree_height + 1; lz++) {
                if (lz >= world->depth) continue;

                for (int ly = ty - 2; ly <= ty + 2; ly++) {
                    if (ly < 0 || ly >= world->height) continue;

                    for (int lx = tx - 2; lx <= tx + 2; lx++) {
                        if (lx < 0 || lx >= world->width) continue;

                        // Distance from trunk
                        int dx = lx - tx;
                        int dy = ly - ty;
                        int dz = lz - (surface_height + tree_height - 1);
                        float dist = sqrtf(dx * dx + dy * dy + dz * dz * 2.0f);

                        // Place leaves in a spherical pattern
                        if (dist < 2.5f && world->blocks[lz][ly][lx] == BLOCK_AIR) {
                            world->blocks[lz][ly][lx] = BLOCK_LEAVES;
                        }
                    }
                }
            }
        }
    }

    // Free heightmap
    free(heightmap);
}

// Generate structures (houses, caves, etc.)
void world_generate_structures(World* world, unsigned int seed) {
    if (!world) return;

    // Set random seed
    set_random_seed(seed + 100);

    // Generate a simple house
    int house_x = world->width / 2;
    int house_y = world->height / 2;
    int house_z = 0;

    // Find ground level
    while (house_z < world->depth &&
        world->blocks[house_z][house_y][house_x] == BLOCK_AIR) {
        house_z++;
    }

    // Build on top of ground
    house_z++;

    // House dimensions
    int house_width = 7;
    int house_length = 9;
    int house_height = 4;

    // Check bounds
    if (house_x + house_width >= world->width ||
        house_y + house_length >= world->height ||
        house_z + house_height >= world->depth) {
        return;
    }

    // Build floor
    for (int y = 0; y < house_length; y++) {
        for (int x = 0; x < house_width; x++) {
            world->blocks[house_z][house_y + y][house_x + x] = BLOCK_WOOD;
        }
    }

    // Build walls
    for (int z = 1; z < house_height; z++) {
        for (int y = 0; y < house_length; y++) {
            for (int x = 0; x < house_width; x++) {
                // Only build walls on the perimeter
                if (x == 0 || x == house_width - 1 || y == 0 || y == house_length - 1) {
                    // Door in the middle of one wall
                    if (!(z < 3 && x == house_width / 2 && y == 0)) {
                        world->blocks[house_z + z][house_y + y][house_x + x] = BLOCK_BRICK;
                    }
                }
            }
        }
    }

    // Build roof
    for (int y = 0; y < house_length; y++) {
        for (int x = 0; x < house_width; x++) {
            world->blocks[house_z + house_height][house_y + y][house_x + x] = BLOCK_WOOD;
        }
    }

    // Add a window
    int window_x = house_width - 2;
    int window_y = house_length - 2;
    int window_z = house_z + 2;

    world->blocks[window_z][house_y + window_y][house_x + window_x] = BLOCK_AIR;
}

// Get block at position
uint8_t world_get_block(World* world, int x, int y, int z) {
    if (!world || !world_is_valid_position(world, x, y, z)) {
        return BLOCK_AIR;
    }

    return world->blocks[z][y][x];
}

// Set block at position
void world_set_block(World* world, int x, int y, int z, uint8_t type) {
    if (!world || !world_is_valid_position(world, x, y, z)) {
        return;
    }

    world->blocks[z][y][x] = type;
}

// Check if a block is solid
int world_is_solid(World* world, int x, int y, int z) {
    if (!world || !world_is_valid_position(world, x, y, z)) {
        return 0;
    }

    uint8_t type = world->blocks[z][y][x];

    if (type >= world->num_block_types) {
        return 0;
    }

    return world->block_types[type].solid;
}

// Check if a position is valid
int world_is_valid_position(World* world, int x, int y, int z) {
    if (!world) {
        return 0;
    }

    return x >= 0 && x < world->width &&
        y >= 0 && y < world->height &&
        z >= 0 && z < world->depth;
}

// Get brightness at position
float world_get_brightness(World* world, int x, int y, int z) {
    if (!world || !world_is_valid_position(world, x, y, z)) {
        return 0.0f;
    }

    // Base brightness from sky
    float brightness = world->sky_brightness;

    // Reduce brightness when underground
    for (int check_z = z + 1; check_z < world->depth; check_z++) {
        if (world_is_valid_position(world, x, y, check_z)) {
            uint8_t block_above = world->blocks[check_z][y][x];
            if (block_above != BLOCK_AIR && block_above != BLOCK_WATER) {
                brightness *= 0.7f;
            }
        }
    }

    return clamp(brightness, 0.2f, 1.0f);
}

// Initialize block types
void world_init_block_types(World* world) {
    if (!world) return;

    if (world->block_types) {
        free(world->block_types);
    }

    world->block_types = (BlockType*)malloc(MAX_BLOCK_TYPES * sizeof(BlockType));
    if (!world->block_types) return;

    // Copy default block types
    memcpy(world->block_types, default_block_types, MAX_BLOCK_TYPES * sizeof(BlockType));
    world->num_block_types = MAX_BLOCK_TYPES;
}

// Get block type information
BlockType world_get_block_type(World* world, uint8_t type) {
    // Use global block types if world is NULL
    if (!world) {
        if (type >= MAX_BLOCK_TYPES) {
            type = 0; // Default to air
        }
        return default_block_types[type];
    }

    if (type >= world->num_block_types) {
        type = 0; // Default to air
    }

    return world->block_types[type];
}

// Save world to a file
int world_save(World* world, const char* filename) {
    if (!world || !filename) return 0;

    FILE* file = fopen(filename, "wb");
    if (!file) return 0;

    // Write dimensions
    fwrite(&world->width, sizeof(int), 1, file);
    fwrite(&world->height, sizeof(int), 1, file);
    fwrite(&world->depth, sizeof(int), 1, file);

    // Write blocks
    for (int z = 0; z < world->depth; z++) {
        for (int y = 0; y < world->height; y++) {
            fwrite(world->blocks[z][y], sizeof(uint8_t), world->width, file);
        }
    }

    // Write time and sky brightness
    fwrite(&world->time_of_day, sizeof(float), 1, file);
    fwrite(&world->sky_brightness, sizeof(float), 1, file);

    fclose(file);
    return 1;
}

// Load world from a file
World* world_load(const char* filename) {
    if (!filename) return NULL;

    FILE* file = fopen(filename, "rb");
    if (!file) return NULL;

    // Read dimensions
    int width, height, depth;
    if (fread(&width, sizeof(int), 1, file) != 1 ||
        fread(&height, sizeof(int), 1, file) != 1 ||
        fread(&depth, sizeof(int), 1, file) != 1) {
        fclose(file);
        return NULL;
    }

    // Create world
    World* world = world_create(width, height, depth);
    if (!world) {
        fclose(file);
        return NULL;
    }

    // Read blocks
    for (int z = 0; z < world->depth; z++) {
        for (int y = 0; y < world->height; y++) {
            if (fread(world->blocks[z][y], sizeof(uint8_t), world->width, file) != world->width) {
                world_destroy(world);
                fclose(file);
                return NULL;
            }
        }
    }

    // Read time and sky brightness
    if (fread(&world->time_of_day, sizeof(float), 1, file) != 1 ||
        fread(&world->sky_brightness, sizeof(float), 1, file) != 1) {
        world_destroy(world);
        fclose(file);
        return NULL;
    }

    fclose(file);
    return world;
}

// Update world lighting
void world_update_lighting(World* world) {
    if (!world) return;

    // Update sky brightness based on time of day
    float time = world->time_of_day;

    // Day/night cycle: 0.0 = midnight, 0.5 = noon
    if (time < 0.25f) {
        // Night to dawn
        world->sky_brightness = smoothstep(0.0f, 0.25f, time) * 0.8f + 0.2f;
    }
    else if (time < 0.5f) {
        // Dawn to noon
        world->sky_brightness = smoothstep(0.25f, 0.5f, time) * 0.2f + 0.8f;
    }
    else if (time < 0.75f) {
        // Noon to dusk
        world->sky_brightness = (1.0f - smoothstep(0.5f, 0.75f, time)) * 0.2f + 0.8f;
    }
    else {
        // Dusk to night
        world->sky_brightness = (1.0f - smoothstep(0.75f, 1.0f, time)) * 0.8f + 0.2f;
    }
}

// Set time of day
void world_set_time(World* world, float time) {
    if (!world) return;

    world->time_of_day = time;
    while (world->time_of_day >= 1.0f) {
        world->time_of_day -= 1.0f;
    }

    world_update_lighting(world);
}
