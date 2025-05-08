/**
 * @file renderer.h
 * @brief Rendering engine for the raycaster
 */
#ifndef RENDERER_H
#define RENDERER_H

#include "world.h"
#include "player.h"

 // Framebuffer structure
typedef struct {
    int width;
    int height;
    char* char_buffer;
    int* fg_color_buffer;
    int* bg_color_buffer;
} Framebuffer;

// Renderer structure
typedef struct {
    Framebuffer* framebuffer;
    int width;
    int height;
    float* depth_buffer;
    int draw_hud;
    int draw_debug;
    int wireframe_mode;
    int show_minimap;
} Renderer;

// Renderer creation and destruction
Renderer* renderer_create(int width, int height);
void renderer_destroy(Renderer* renderer);

// Rendering functions
void renderer_clear(Renderer* renderer);
void renderer_render_world(Renderer* renderer, World* world, Player* player);
void renderer_render_hud(Renderer* renderer, Player* player, World* world);
void renderer_render_debug(Renderer* renderer, Player* player);
void renderer_render_minimap(Renderer* renderer, World* world, Player* player);
void renderer_present(Renderer* renderer);

// Utility functions
void renderer_set_pixel(Renderer* renderer, int x, int y, char c, int fg, int bg);
void renderer_draw_line(Renderer* renderer, int x1, int y1, int x2, int y2, char c, int fg, int bg);
void renderer_draw_rect(Renderer* renderer, int x, int y, int w, int h, char c, int fg, int bg);
void renderer_draw_text(Renderer* renderer, int x, int y, const char* text, int fg, int bg);
void renderer_toggle_hud(Renderer* renderer);
void renderer_toggle_debug(Renderer* renderer);
void renderer_toggle_wireframe(Renderer* renderer);
void renderer_toggle_minimap(Renderer* renderer);

#endif /* RENDERER_H */
