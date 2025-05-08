/**
 * @file player.c
 * @brief Implementation of player state and physics
 */
#include "player.h"
#include "config.h"
#include "raycaster.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

 // Create a new player
Player* player_create(void) {
    Player* player = (Player*)malloc(sizeof(Player));
    if (!player) return NULL;

    // Initialize position and rotation
    player->position = vec3_create(WORLD_WIDTH / 2.0f, WORLD_HEIGHT / 2.0f, GROUND_HEIGHT + EYE_HEIGHT);
    player->velocity = vec3_create(0.0f, 0.0f, 0.0f);
    player->rotation = vec2_create(0.0f, 0.0f);

    // Initialize other properties
    player->height = PLAYER_HEIGHT;
    player->width = PLAYER_WIDTH;
    player->grounded = 0;
    player->flying = 0;
    player->health = 100.0f;
    player->stamina = 100.0f;
    player->selected_slot = 1;

    // Initialize inventory
    player->inventory = (uint8_t*)malloc(MAX_BLOCK_TYPES * sizeof(uint8_t));
    if (!player->inventory) {
        free(player);
        return NULL;
    }

    // Set initial inventory
    memset(player->inventory, 0, MAX_BLOCK_TYPES * sizeof(uint8_t));
    player->inventory[BLOCK_DIRT] = 64;
    player->inventory[BLOCK_STONE] = 64;
    player->inventory[BLOCK_GRASS] = 64;
    player->inventory[BLOCK_WOOD] = 16;
    player->inventory[BLOCK_BRICK] = 16;

    return player;
}

// Destroy a player
void player_destroy(Player* player) {
    if (!player) return;

    free(player->inventory);
    free(player);
}

// Update player state
void player_update(Player* player, World* world, float delta_time) {
    if (!player || !world) return;

    // Apply gravity if not flying
    if (!player->flying) {
        player->velocity.z -= GRAVITY * delta_time;
    }
    else {
        // Slow down in flying mode
        player->velocity.z *= 0.9f;
    }

    // Update position
    Vector3 new_pos = vec3_add(player->position, vec3_mul(player->velocity, delta_time));

    // Check for collision with ground
    int ground_x = (int)floorf(new_pos.x);
    int ground_y = (int)floorf(new_pos.y);
    int ground_z = (int)floorf(new_pos.z - player->height);

    player->grounded = 0;

    // Check if we hit the ground
    if (ground_z >= 0 && ground_z < world->depth) {
        if (world_is_solid(world, ground_x, ground_y, ground_z)) {
            new_pos.z = ground_z + 1.0f + player->height;
            player->velocity.z = 0.0f;
            player->grounded = 1;
        }
    }

    // Check for collision with walls
    int wall_x = (int)floorf(new_pos.x);
    int wall_y = (int)floorf(new_pos.y);
    int wall_z = (int)floorf(new_pos.z);

    // Check collision in the X direction
    if (world_is_solid(world, wall_x, (int)floorf(player->position.y), wall_z)) {
        new_pos.x = player->position.x;
        player->velocity.x = 0.0f;
    }

    // Check collision in the Y direction
    if (world_is_solid(world, (int)floorf(new_pos.x), wall_y, wall_z)) {
        new_pos.y = player->position.y;
        player->velocity.y = 0.0f;
    }

    // Check collision with ceiling
    int ceiling_z = (int)floorf(new_pos.z);
    if (ceiling_z < world->depth && world_is_solid(world, wall_x, wall_y, ceiling_z)) {
        new_pos.z = player->position.z;
        player->velocity.z = 0.0f;
    }

    // Update position
    player->position = new_pos;

    // Limit max movement speed
    float speed_limit = 10.0f;
    float current_speed = vec3_length(player->velocity);
    if (current_speed > speed_limit) {
        player->velocity = vec3_mul(vec3_normalize(player->velocity), speed_limit);
    }

    // Apply friction
    if (player->grounded) {
        player->velocity.x *= 0.8f;
        player->velocity.y *= 0.8f;
    }
    else {
        player->velocity.x *= 0.98f;
        player->velocity.y *= 0.98f;
    }

    // Make sure player stays within world bounds
    if (player->position.x < 1.0f) player->position.x = 1.0f;
    if (player->position.y < 1.0f) player->position.y = 1.0f;
    if (player->position.x > world->width - 1.0f) player->position.x = world->width - 1.0f;
    if (player->position.y > world->height - 1.0f) player->position.y = world->height - 1.0f;

    // Recover stamina slowly
    player->stamina = min_float(player->stamina + 0.1f, 100.0f);
}

// Move player
void player_move(Player* player, World* world, float forward, float right, float delta_time) {
    if (!player) return;

    // Get forward and right vectors
    Vector3 forward_vec = player_get_forward_vector(player);
    Vector3 right_vec = player_get_right_vector(player);

    // Remove vertical component for horizontal movement
    forward_vec.z = 0.0f;
    right_vec.z = 0.0f;

    // Normalize vectors
    forward_vec = vec3_normalize(forward_vec);
    right_vec = vec3_normalize(right_vec);

    // Calculate movement direction
    Vector3 move_dir = vec3_create(0.0f, 0.0f, 0.0f);

    if (forward != 0.0f) {
        move_dir = vec3_add(move_dir, vec3_mul(forward_vec, forward));
    }

    if (right != 0.0f) {
        move_dir = vec3_add(move_dir, vec3_mul(right_vec, right));
    }

    // Normalize if moving diagonally
    if (vec3_length(move_dir) > 0.0f) {
        move_dir = vec3_normalize(move_dir);
    }

    // Apply movement
    float speed = player->flying ? PLAYER_SPEED * 2.0f : PLAYER_SPEED;

    // Running uses stamina
    if (delta_time > 0.0f && vec3_length(move_dir) > 0.0f) {
        if (player->grounded && player->stamina > 0.0f) {
            // Use stamina when running
            player->stamina -= 0.2f;
            if (player->stamina < 0.0f) player->stamina = 0.0f;
        }
    }

    // Apply movement to velocity
    player->velocity.x += move_dir.x * speed;
    player->velocity.y += move_dir.y * speed;
}

// Make player jump
void player_jump(Player* player) {
    if (!player) return;

    if (player->grounded) {
        player->velocity.z = JUMP_FORCE;
        player->grounded = 0;
    }
    else if (player->flying) {
        player->velocity.z = JUMP_FORCE * 0.5f;
    }
}

// Rotate player view
void player_rotate(Player* player, float pitch, float yaw) {
    if (!player) return;

    // Update rotation
    player->rotation.x += pitch;
    player->rotation.y += yaw;

    // Clamp pitch to prevent flipping
    if (player->rotation.x > 1.5f) player->rotation.x = 1.5f;
    if (player->rotation.x < -1.5f) player->rotation.x = -1.5f;

    // Wrap yaw around
    while (player->rotation.y >= 2.0f * M_PI) {
        player->rotation.y -= 2.0f * M_PI;
    }

    while (player->rotation.y < 0.0f) {
        player->rotation.y += 2.0f * M_PI;
    }
}

// Check if player is colliding with the world
int player_is_colliding(Player* player, World* world) {
    if (!player || !world) return 0;

    // Check if inside a block
    int x = (int)floorf(player->position.x);
    int y = (int)floorf(player->position.y);
    int z = (int)floorf(player->position.z);

    return world_is_solid(world, x, y, z);
}

// Get player camera position
Vector3 player_get_camera_position(Player* player) {
    if (!player) return vec3_create(0.0f, 0.0f, 0.0f);

    // Camera is at player's eye level
    return player->position;
}

// Get player view direction
Vector3 player_get_view_direction(Player* player) {
    if (!player) return vec3_create(0.0f, 0.0f, 1.0f);

    // Convert rotation to direction
    return vec3_from_angles(player->rotation.x, player->rotation.y);
}

// Get player forward vector (horizontal only)
Vector3 player_get_forward_vector(Player* player) {
    if (!player) return vec3_create(0.0f, 0.0f, 0.0f);

    // Get forward direction (horizontal only)
    float yaw = player->rotation.y;
    return vec3_create(cosf(yaw), sinf(yaw), 0.0f);
}

// Get player right vector (horizontal only)
Vector3 player_get_right_vector(Player* player) {
    if (!player) return vec3_create(0.0f, 0.0f, 0.0f);

    // Get right direction (horizontal only)
    float yaw = player->rotation.y + M_PI / 2.0f;
    return vec3_create(cosf(yaw), sinf(yaw), 0.0f);
}

// Get player up vector
Vector3 player_get_up_vector(Player* player) {
    // Up is always along Z axis
    return vec3_create(0.0f, 0.0f, 1.0f);
}

// Interact with the world
void player_interact(Player* player, World* world) {
    if (!player || !world) return;

    // Cast a ray from player
    Vector3 pos = player_get_camera_position(player);
    Vector3 dir = player_get_view_direction(player);

    RayHit hit = cast_ray(world, pos, dir, 5.0f);

    if (hit.hit) {
        // Do something with the hit
        // For example, print block info
        BlockType block_type = world_get_block_type(world, hit.block_type);
        // Interaction logic here
    }
}

// Place a block in the world
void player_place_block(Player* player, World* world, uint8_t block_type) {
    if (!player || !world) return;

    // Cast a ray from player
    Vector3 pos = player_get_camera_position(player);
    Vector3 dir = player_get_view_direction(player);

    RayHit hit = cast_ray(world, pos, dir, 5.0f);

    if (hit.hit) {
        // Place block adjacent to hit position
        int x = (int)floorf(hit.position.x + hit.normal.x * 0.5f);
        int y = (int)floorf(hit.position.y + hit.normal.y * 0.5f);
        int z = (int)floorf(hit.position.z + hit.normal.z * 0.5f);

        // Check if the position is valid
        if (world_is_valid_position(world, x, y, z)) {
            // Check if player has enough blocks
            if (player->inventory[block_type] > 0) {
                // Place block
                world_set_block(world, x, y, z, block_type);

                // Reduce inventory
                player->inventory[block_type]--;
            }
        }
    }
}

// Break a block in the world
void player_break_block(Player* player, World* world) {
    if (!player || !world) return;

    // Cast a ray from player
    Vector3 pos = player_get_camera_position(player);
    Vector3 dir = player_get_view_direction(player);

    RayHit hit = cast_ray(world, pos, dir, 5.0f);

    if (hit.hit) {
        // Get block position
        int x = (int)floorf(hit.position.x);
        int y = (int)floorf(hit.position.y);
        int z = (int)floorf(hit.position.z);

        // Add block to inventory
        uint8_t block_type = world_get_block(world, x, y, z);
        player->inventory[block_type]++;

        // Remove block
        world_set_block(world, x, y, z, BLOCK_AIR);
    }
}

// Cast a ray and get the block position and normal
int player_raycast_block(Player* player, World* world, Vector3* out_position, Vector3* out_normal) {
    if (!player || !world) return 0;

    // Cast a ray from player
    Vector3 pos = player_get_camera_position(player);
    Vector3 dir = player_get_view_direction(player);

    RayHit hit = cast_ray(world, pos, dir, 5.0f);

    if (hit.hit) {
        if (out_position) *out_position = hit.position;
        if (out_normal) *out_normal = hit.normal;
        return 1;
    }

    return 0;
}

// Select an inventory slot
void player_select_slot(Player* player, int slot) {
    if (!player) return;

    if (slot >= 1 && slot <= 9) {
        player->selected_slot = slot;
    }
}

// Get the selected block type
uint8_t player_get_selected_block(Player* player) {
    if (!player) return BLOCK_AIR;

    return player->selected_slot;
}

// Give blocks to the player
void player_give_block(Player* player, uint8_t block_type, int amount) {
    if (!player) return;

    if (block_type < MAX_BLOCK_TYPES) {
        player->inventory[block_type] += amount;
    }
}
