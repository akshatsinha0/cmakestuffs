/**
 * @file renderer.c
 * @brief Implementation of the renderer
 */

 /* Add warning suppression at the beginning of the file */
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4996) /* Disable sprintf warnings */
#pragma warning(disable:4267) /* Disable size_t to int conversion warnings */
#pragma warning(disable:4244) /* Disable double to float conversion warnings */
#endif

#include "renderer.h"
#include "config.h"
#include "terminal.h"
#include "raycaster.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

// Create a new renderer
Renderer* renderer_create(int width, int height) {
    Renderer* renderer = (Renderer*)malloc(sizeof(Renderer));
    if (!renderer) return NULL;

    // Set dimensions
    renderer->width = width;
    renderer->height = height;

    // Create framebuffer
    renderer->framebuffer = (Framebuffer*)malloc(sizeof(Framebuffer));
    if (!renderer->framebuffer) {
        free(renderer);
        return NULL;
    }

    // Initialize framebuffer
    renderer->framebuffer->width = width;
    renderer->framebuffer->height = height;
    renderer->framebuffer->char_buffer = (char*)malloc(width * height * sizeof(char));
    renderer->framebuffer->fg_color_buffer = (int*)malloc(width * height * sizeof(int));
    renderer->framebuffer->bg_color_buffer = (int*)malloc(width * height * sizeof(int));

    // Check allocations
    if (!renderer->framebuffer->char_buffer ||
        !renderer->framebuffer->fg_color_buffer ||
        !renderer->framebuffer->bg_color_buffer) {
        renderer_destroy(renderer);
        return NULL;
    }

    // Create depth buffer
    renderer->depth_buffer = (float*)malloc(width * height * sizeof(float));
    if (!renderer->depth_buffer) {
        renderer_destroy(renderer);
        return NULL;
    }

    // Set default options
    renderer->draw_hud = 1;
    renderer->draw_debug = 0;
    renderer->wireframe_mode = 0;
    renderer->show_minimap = 1;

    return renderer;
}

// Destroy a renderer
void renderer_destroy(Renderer* renderer) {
    if (!renderer) return;

    // Free framebuffer
    if (renderer->framebuffer) {
        free(renderer->framebuffer->char_buffer);
        free(renderer->framebuffer->fg_color_buffer);
        free(renderer->framebuffer->bg_color_buffer);
        free(renderer->framebuffer);
    }

    // Free depth buffer
    free(renderer->depth_buffer);

    // Free renderer
    free(renderer);
}

// Clear the renderer
void renderer_clear(Renderer* renderer) {
    if (!renderer) return;

    // Clear framebuffer
    Framebuffer* fb = renderer->framebuffer;
    memset(fb->char_buffer, ' ', fb->width * fb->height * sizeof(char));
    memset(fb->fg_color_buffer, COLOR_WHITE, fb->width * fb->height * sizeof(int));
    memset(fb->bg_color_buffer, COLOR_BLACK, fb->width * fb->height * sizeof(int));

    // Clear depth buffer
    for (int i = 0; i < renderer->width * renderer->height; i++) {
        renderer->depth_buffer[i] = INFINITY;
    }
}

// Render the world
void renderer_render_world(Renderer* renderer, World* world, Player* player) {
    if (!renderer || !world || !player) return;

    // Get camera position and direction
    Vector3 camera_pos = player_get_camera_position(player);
    Vector3 camera_dir = player_get_view_direction(player);

    // Get up and right vectors
    Vector3 camera_up = vec3_create(0.0f, 0.0f, 1.0f);
    Vector3 camera_right = vec3_cross(camera_dir, camera_up);
    camera_up = vec3_cross(camera_right, camera_dir);

    // Normalize vectors
    camera_dir = vec3_normalize(camera_dir);
    camera_up = vec3_normalize(camera_up);
    camera_right = vec3_normalize(camera_right);

    // Get screen dimensions
    int screen_width = renderer->width;
    int screen_height = renderer->height;

    // Aspect ratio
    float aspect_ratio = (float)screen_width / screen_height;

    // Render sky
    float sky_brightness = world->sky_brightness;
    int sky_color = (sky_brightness > 0.5f) ? COLOR_CYAN : COLOR_BLACK;

    // Render each pixel
    for (int y = 0; y < screen_height; y++) {
        for (int x = 0; x < screen_width; x++) {
            // Calculate ray direction
            float screen_x = (2.0f * x / screen_width - 1.0f) * aspect_ratio * FOV_HORIZONTAL;
            float screen_y = (1.0f - 2.0f * y / screen_height) * FOV_VERTICAL;

            // Ray direction
            Vector3 ray_dir;
            ray_dir.x = camera_dir.x + screen_x * camera_right.x + screen_y * camera_up.x;
            ray_dir.y = camera_dir.y + screen_x * camera_right.y + screen_y * camera_up.y;
            ray_dir.z = camera_dir.z + screen_x * camera_right.z + screen_y * camera_up.z;
            ray_dir = vec3_normalize(ray_dir);

            // Cast ray
            RayHit hit = cast_ray(world, camera_pos, ray_dir, FAR_PLANE);

            // Render hit or sky
            if (hit.hit) {
                // Calculate buffer index
                int index = y * screen_width + x;

                // Check depth buffer
                if (hit.distance < renderer->depth_buffer[index]) {
                    // Update depth buffer
                    renderer->depth_buffer[index] = hit.distance;

                    // Get display character
                    char display_char = get_hit_display_char(hit);

                    // Get color
                    int fg_color = get_hit_color(hit);
                    int bg_color = COLOR_BLACK;

                    // Write to framebuffer
                    renderer->framebuffer->char_buffer[index] = display_char;
                    renderer->framebuffer->fg_color_buffer[index] = fg_color;
                    renderer->framebuffer->bg_color_buffer[index] = bg_color;
                }
            }
            else {
                // Draw sky gradient
                int index = y * screen_width + x;
                float sky_y = (float)y / screen_height;

                char display_char = ' ';
                int fg_color = COLOR_BLACK;
                int bg_color;

                if (sky_y < 0.5f) {
                    // Upper sky (darker)
                    bg_color = sky_color;
                }
                else {
                    // Lower sky (lighter)
                    bg_color = (sky_color == COLOR_CYAN) ? COLOR_BLUE : COLOR_BLACK;
                }

                // Write to framebuffer
                renderer->framebuffer->char_buffer[index] = display_char;
                renderer->framebuffer->fg_color_buffer[index] = fg_color;
                renderer->framebuffer->bg_color_buffer[index] = bg_color;
            }
        }
    }
}

// Render the HUD (heads-up display)
void renderer_render_hud(Renderer* renderer, Player* player, World* world) {
    if (!renderer || !player || !world || !renderer->draw_hud) return;

    // Draw crosshair in the center
    renderer_set_pixel(renderer,
        renderer->width / 2,
        renderer->height / 2,
        '+', COLOR_WHITE, COLOR_BLACK);

    // Draw inventory bar at the bottom
    int inventory_width = 9;  // 9 slots
    int inventory_x = (renderer->width - inventory_width) / 2;
    int inventory_y = renderer->height - 2;

    // Background for inventory
    for (int i = 0; i < inventory_width; i++) {
        renderer_set_pixel(renderer, inventory_x + i, inventory_y,
            '[', COLOR_WHITE, COLOR_BLACK);
    }

    // Show selected slot
    int selected_x = inventory_x + player->selected_slot - 1;
    renderer_set_pixel(renderer, selected_x, inventory_y,
        '*', COLOR_WHITE | COLOR_BRIGHT, COLOR_BLACK);

    // Show coordinates
    char coords[32];
    sprintf(coords, "X:%.1f Y:%.1f Z:%.1f",
        player->position.x, player->position.y, player->position.z);
    renderer_draw_text(renderer, 2, 1, coords, COLOR_WHITE, COLOR_BLACK);

    // Show health and stamina
    char stats[32];
    sprintf(stats, "HP:%.0f SP:%.0f", player->health, player->stamina);
    renderer_draw_text(renderer, 2, 2, stats, COLOR_WHITE, COLOR_BLACK);

    // Show time of day
    char time[32];
    sprintf(time, "Time: %.2f", world->time_of_day);
    renderer_draw_text(renderer, renderer->width - 13, 1, time, COLOR_WHITE, COLOR_BLACK);

    // Controls help
    renderer_draw_text(renderer, 2, renderer->height - 2,
        "WASD: Look | IJKL: Move | Space: Jump | F: Fly | E: Place | Q: Break",
        COLOR_WHITE, COLOR_BLACK);
}

// Render debug information
void renderer_render_debug(Renderer* renderer, Player* player) {
    if (!renderer || !player || !renderer->draw_debug) return;

    // Debug info
    char debug[128];

    // Show player velocity
    sprintf(debug, "VEL: X:%.2f Y:%.2f Z:%.2f",
        player->velocity.x, player->velocity.y, player->velocity.z);
    renderer_draw_text(renderer, 2, 4, debug, COLOR_WHITE, COLOR_BLACK);

    // Show player rotation
    sprintf(debug, "ROT: P:%.2f Y:%.2f",
        player->rotation.x * 180.0f / M_PI, player->rotation.y * 180.0f / M_PI);
    renderer_draw_text(renderer, 2, 5, debug, COLOR_WHITE, COLOR_BLACK);

    // Show grounded state
    sprintf(debug, "GROUNDED: %s | FLYING: %s",
        player->grounded ? "YES" : "NO", player->flying ? "YES" : "NO");
    renderer_draw_text(renderer, 2, 6, debug, COLOR_WHITE, COLOR_BLACK);
}

// Render minimap
void renderer_render_minimap(Renderer* renderer, World* world, Player* player) {
    if (!renderer || !world || !player || !renderer->show_minimap) return;

    // Minimap size and position
    int map_size = 16;
    int map_x = renderer->width - map_size - 2;
    int map_y = 2;

    // Minimap border
    renderer_draw_rect(renderer, map_x - 1, map_y - 1, map_size + 2, map_size + 2, ' ', COLOR_WHITE, COLOR_BLACK);

    // Draw minimap
    int player_x = (int)player->position.x;
    int player_y = (int)player->position.y;

    // Draw terrain
    for (int y = 0; y < map_size; y++) {
        for (int x = 0; x < map_size; x++) {
            int world_x = player_x + x - map_size / 2;
            int world_y = player_y + y - map_size / 2;

            if (world_x < 0 || world_x >= world->width ||
                world_y < 0 || world_y >= world->height) {
                continue;
            }

            // Find highest non-air block at this position
            int highest_z = -1;
            uint8_t highest_block = BLOCK_AIR;

            for (int z = world->depth - 1; z >= 0; z--) {
                uint8_t block = world_get_block(world, world_x, world_y, z);
                if (block != BLOCK_AIR) {
                    highest_z = z;
                    highest_block = block;
                    break;
                }
            }

            if (highest_z >= 0) {
                // Get block type info
                BlockType block_type = world_get_block_type(world, highest_block);

                // Use block_type to avoid 'unreferenced local variable' warning
                renderer_set_pixel(renderer, map_x + x, map_y + y,
                    block_type.display_char, block_type.fg_color, COLOR_BLACK);
            }
        }
    }

    // Draw player position
    renderer_set_pixel(renderer, map_x + map_size / 2, map_y + map_size / 2,
        'P', COLOR_WHITE | COLOR_BRIGHT, COLOR_BLACK);
}

// Present the rendered frame
void renderer_present(Renderer* renderer) {
    if (!renderer) return;

    // Set cursor to top-left
    terminal_set_cursor(0, 0);

    // Draw framebuffer
    Framebuffer* fb = renderer->framebuffer;
    for (int y = 0; y < fb->height; y++) {
        for (int x = 0; x < fb->width; x++) {
            int index = y * fb->width + x;

            char c = fb->char_buffer[index];
            int fg = fb->fg_color_buffer[index];
            int bg = fb->bg_color_buffer[index];

            terminal_set_color(fg, bg);
            putchar(c);
        }
        putchar('\n');
    }

    // Reset terminal color
    terminal_reset_color();
    terminal_flush();
}

// Set a pixel in the framebuffer
void renderer_set_pixel(Renderer* renderer, int x, int y, char c, int fg, int bg) {
    if (!renderer) return;

    Framebuffer* fb = renderer->framebuffer;

    // Check bounds
    if (x < 0 || x >= fb->width || y < 0 || y >= fb->height) {
        return;
    }

    // Set pixel
    int index = y * fb->width + x;
    fb->char_buffer[index] = c;
    fb->fg_color_buffer[index] = fg;
    fb->bg_color_buffer[index] = bg;
}

// Draw a line
void renderer_draw_line(Renderer* renderer, int x1, int y1, int x2, int y2, char c, int fg, int bg) {
    if (!renderer) return;

    // Bresenham's line algorithm
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        renderer_set_pixel(renderer, x1, y1, c, fg, bg);

        if (x1 == x2 && y1 == y2) break;

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

// Draw a rectangle
void renderer_draw_rect(Renderer* renderer, int x, int y, int w, int h, char c, int fg, int bg) {
    if (!renderer) return;

    // Draw horizontal lines
    for (int i = 0; i < w; i++) {
        renderer_set_pixel(renderer, x + i, y, c, fg, bg);
        renderer_set_pixel(renderer, x + i, y + h - 1, c, fg, bg);
    }

    // Draw vertical lines
    for (int i = 0; i < h; i++) {
        renderer_set_pixel(renderer, x, y + i, c, fg, bg);
        renderer_set_pixel(renderer, x + w - 1, y + i, c, fg, bg);
    }
}

// Draw text
void renderer_draw_text(Renderer* renderer, int x, int y, const char* text, int fg, int bg) {
    if (!renderer || !text) return;

    // Draw each character
    int len = (int)strlen(text); // Added cast to fix size_t to int warning
    for (int i = 0; i < len; i++) {
        renderer_set_pixel(renderer, x + i, y, text[i], fg, bg);
    }
}

// Toggle HUD
void renderer_toggle_hud(Renderer* renderer) {
    if (!renderer) return;

    renderer->draw_hud = !renderer->draw_hud;
}

// Toggle debug info
void renderer_toggle_debug(Renderer* renderer) {
    if (!renderer) return;

    renderer->draw_debug = !renderer->draw_debug;
}

// Toggle wireframe mode
void renderer_toggle_wireframe(Renderer* renderer) {
    if (!renderer) return;

    renderer->wireframe_mode = !renderer->wireframe_mode;
}

// Toggle minimap
void renderer_toggle_minimap(Renderer* renderer) {
    if (!renderer) return;

    renderer->show_minimap = !renderer->show_minimap;
}

/* Restore warning settings */
#ifdef _MSC_VER
#pragma warning(pop)
#endif
