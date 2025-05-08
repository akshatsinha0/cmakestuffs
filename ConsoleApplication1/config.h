/**
 * @file config.h
 * @brief Configuration constants for the raycasting engine
 */
#define _USE_MATH_DEFINES
#include <math.h>
#define _CRT_SECURE_NO_WARNINGS

#ifndef CONFIG_H
#define CONFIG_H

 // Screen configuration
#define SCREEN_WIDTH 120
#define SCREEN_HEIGHT 40
#define SCREEN_TITLE "Voxel Explorer - ASCII Raycasting Engine"

// World configuration
#define WORLD_WIDTH 64
#define WORLD_HEIGHT 64
#define WORLD_DEPTH 16
#define GROUND_HEIGHT 3

// View configuration
#define FOV_HORIZONTAL 1.0f
#define FOV_VERTICAL 0.7f
#define EYE_HEIGHT 1.6f
#define NEAR_PLANE 0.01f
#define FAR_PLANE 20.0f
#define EDGE_THRESHOLD 0.03f

// Physics configuration
#define GRAVITY 0.05f
#define JUMP_FORCE 0.4f
#define PLAYER_SPEED 0.15f
#define PLAYER_TURN_SPEED 0.05f
#define PLAYER_HEIGHT 1.8f
#define PLAYER_WIDTH 0.6f

// Game configuration
#define MAX_ENTITIES 64
#define MAX_ITEMS 32
#define MAX_BLOCK_TYPES 16
#define SAVE_FILE "world.dat"

// Rendering configuration
#define ENABLE_COLORS 1
#define ENABLE_SHADING 1
#define ENABLE_FOG 1
#define RENDER_DISTANCE 16
#define FOG_START 10.0f
#define FOG_END 15.0f

// Time configuration
#define TARGET_FPS 30
#define MS_PER_FRAME (1000 / TARGET_FPS)

// Debugging
#define DEBUG_MODE 0

// Block type IDs
#define BLOCK_AIR 0
#define BLOCK_DIRT 1
#define BLOCK_GRASS 2
#define BLOCK_STONE 3
#define BLOCK_WOOD 4
#define BLOCK_LEAVES 5
#define BLOCK_WATER 6
#define BLOCK_SAND 7
#define BLOCK_BRICK 8

#define COLOR_BLACK 0
#define COLOR_RED 1
#define COLOR_GREEN 2
#define COLOR_YELLOW 3
#define COLOR_BLUE 4
#define COLOR_MAGENTA 5
#define COLOR_CYAN 6
#define COLOR_WHITE 7
#define COLOR_BRIGHT 8
#endif /* CONFIG_H */
