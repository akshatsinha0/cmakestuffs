/**
 * @file player.h
 * @brief Player state and physics
 */
#ifndef PLAYER_H
#define PLAYER_H

#include "vector.h"
#include "world.h"

 // Player structure
typedef struct {
    Vector3 position;    // Position in the world
    Vector3 velocity;    // Current velocity
    Vector2 rotation;    // Pitch and yaw (in radians)
    float height;        // Player height
    float width;         // Player width
    int grounded;        // Whether player is on the ground
    int flying;          // Whether player is flying
    float health;        // Player health
    float stamina;       // Player stamina
    int selected_slot;   // Currently selected inventory slot
    uint8_t* inventory;  // Player inventory
} Player;

// Player creation and destruction
Player* player_create(void);
void player_destroy(Player* player);

// Movement and physics
void player_update(Player* player, World* world, float delta_time);
void player_move(Player* player, World* world, float forward, float right, float delta_time);
void player_jump(Player* player);
void player_rotate(Player* player, float pitch, float yaw);
int player_is_colliding(Player* player, World* world);

// Camera and view
Vector3 player_get_camera_position(Player* player);
Vector3 player_get_view_direction(Player* player);
Vector3 player_get_forward_vector(Player* player);
Vector3 player_get_right_vector(Player* player);
Vector3 player_get_up_vector(Player* player);

// Interaction
void player_interact(Player* player, World* world);
void player_place_block(Player* player, World* world, uint8_t block_type);
void player_break_block(Player* player, World* world);
int player_raycast_block(Player* player, World* world, Vector3* out_position, Vector3* out_normal);

// Inventory
void player_select_slot(Player* player, int slot);
uint8_t player_get_selected_block(Player* player);
void player_give_block(Player* player, uint8_t block_type, int amount);

#endif /* PLAYER_H */
