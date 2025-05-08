/**
 * @file main.c
 * @brief Main entry point for the ASCII raycasting engine
 */
#include "config.h"
#include "terminal.h"
#include "world.h"
#include "player.h"
#include "renderer.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

 // Global game state
typedef struct {
    int running;
    int paused;
    World* world;
    Player* player;
    Renderer* renderer;
    unsigned long long last_frame_time;
    float frame_time;
    int frame_count;
    float fps;
    unsigned long long fps_time;
} GameState;

// Function prototypes
void game_init(GameState* game);
void game_cleanup(GameState* game);
void game_update(GameState* game);
void game_render(GameState* game);
void game_process_input(GameState* game);
void handle_resize(GameState* game);
void show_title_screen(GameState* game);
void show_pause_menu(GameState* game);

// Entry point
int main(void) {
    // Seed random number generator
    srand((unsigned int)time(NULL));

    // Create game state
    GameState game = { 0 };
    game.running = 1;
    game.paused = 0;

    // Initialize game
    game_init(&game);

    // Show title screen
    show_title_screen(&game);

    // Main game loop
    while (game.running) {
        // Calculate frame time
        unsigned long long current_time = get_time_ms();
        game.frame_time = (current_time - game.last_frame_time) / 1000.0f;
        game.last_frame_time = current_time;

        // Update FPS counter every second
        game.frame_count++;
        if (current_time - game.fps_time >= 1000) {
            game.fps = game.frame_count * 1000.0f / (current_time - game.fps_time);
            game.frame_count = 0;
            game.fps_time = current_time;
        }

        // Process input
        game_process_input(&game);

        // Update game state if not paused
        if (!game.paused) {
            game_update(&game);
        }

        // Render
        game_render(&game);

        // Cap frame rate
        unsigned long long frame_time = get_time_ms() - current_time;
        if (frame_time < MS_PER_FRAME) {
            terminal_sleep(MS_PER_FRAME - frame_time);
        }
    }

    // Cleanup
    game_cleanup(&game);

    return 0;
}

// Initialize game
void game_init(GameState* game) {
    if (!game) return;

    // Initialize terminal
    if (!terminal_init()) {
        fprintf(stderr, "Failed to initialize terminal\n");
        exit(1);
    }

    // Get terminal size
    int width, height;
    terminal_get_size(&width, &height);

    // Create renderer
    game->renderer = renderer_create(width, height);
    if (!game->renderer) {
        fprintf(stderr, "Failed to create renderer\n");
        terminal_cleanup();
        exit(1);
    }

    // Create world
    game->world = world_create(WORLD_WIDTH, WORLD_HEIGHT, WORLD_DEPTH);
    if (!game->world) {
        fprintf(stderr, "Failed to create world\n");
        renderer_destroy(game->renderer);
        terminal_cleanup();
        exit(1);
    }

    // Initialize block types
    world_init_block_types(game->world);

    // Generate world
    world_generate_terrain(game->world, (unsigned int)time(NULL));
    world_generate_structures(game->world, (unsigned int)time(NULL) + 100);

    // Create player
    game->player = player_create();
    if (!game->player) {
        fprintf(stderr, "Failed to create player\n");
        world_destroy(game->world);
        renderer_destroy(game->renderer);
        terminal_cleanup();
        exit(1);
    }

    // Initialize player position
    game->player->position.x = WORLD_WIDTH / 2.0f;
    game->player->position.y = WORLD_HEIGHT / 2.0f;

    // Find a safe position on the ground
    for (int z = WORLD_DEPTH - 1; z >= 0; z--) {
        int x = (int)game->player->position.x;
        int y = (int)game->player->position.y;

        if (z < WORLD_DEPTH - 1 &&
            world_is_solid(game->world, x, y, z) &&
            !world_is_solid(game->world, x, y, z + 1) &&
            !world_is_solid(game->world, x, y, z + 2)) {

            game->player->position.z = z + 1.0f + EYE_HEIGHT;
            break;
        }
    }

    // Initialize timing
    game->last_frame_time = get_time_ms();
    game->fps_time = game->last_frame_time;
    game->frame_count = 0;
    game->fps = 0.0f;

    // Set world time
    world_set_time(game->world, 0.5f); // Start at noon
}

// Cleanup game
void game_cleanup(GameState* game) {
    if (!game) return;

    // Destroy game objects
    if (game->player) player_destroy(game->player);
    if (game->world) world_destroy(game->world);
    if (game->renderer) renderer_destroy(game->renderer);

    // Cleanup terminal
    terminal_cleanup();
}

// Update game state
void game_update(GameState* game) {
    if (!game) return;

    // Update world time
    float time_speed = 0.001f;
    world_set_time(game->world, game->world->time_of_day + time_speed * game->frame_time);

    // Update lighting
    world_update_lighting(game->world);

    // Update player
    player_update(game->player, game->world, game->frame_time);
}

// Render game
void game_render(GameState* game) {
    if (!game) return;

    // Clear renderer
    renderer_clear(game->renderer);

    if (game->paused) {
        // Show pause menu
        show_pause_menu(game);
    }
    else {
        // Render world
        renderer_render_world(game->renderer, game->world, game->player);

        // Render HUD
        renderer_render_hud(game->renderer, game->player, game->world);

        // Render debug info if enabled
        renderer_render_debug(game->renderer, game->player);

        // Render minimap
        renderer_render_minimap(game->renderer, game->world, game->player);

        // Show FPS
        char fps_text[32];
        sprintf(fps_text, "FPS: %.1f", game->fps);
        renderer_draw_text(game->renderer, game->renderer->width - 12, 2, fps_text, COLOR_WHITE, COLOR_BLACK);
    }

    // Present frame
    renderer_present(game->renderer);
}

// Process input
void game_process_input(GameState* game) {
    if (!game) return;

    // Process input
    terminal_process_input();

    // Check for quit
    if (terminal_key_pressed('q')) {
        game->running = 0;
        return;
    }

    // Toggle pause
    if (terminal_key_pressed('p')) {
        game->paused = !game->paused;
        return;
    }

    if (game->paused) {
        // Pause menu input
        if (terminal_key_pressed('r')) {
            game->paused = 0;
        }
    }
    else {
        // Game input

        // Player movement
        float forward = 0.0f;
        float right = 0.0f;

        if (terminal_key_held('i')) forward += 1.0f;
        if (terminal_key_held('k')) forward -= 1.0f;
        if (terminal_key_held('l')) right += 1.0f;
        if (terminal_key_held('j')) right -= 1.0f;

        player_move(game->player, game->world, forward, right, game->frame_time);

        // Player looking
        float turn_speed = PLAYER_TURN_SPEED;

        if (terminal_key_held('w')) player_rotate(game->player, turn_speed, 0.0f);
        if (terminal_key_held('s')) player_rotate(game->player, -turn_speed, 0.0f);
        if (terminal_key_held('a')) player_rotate(game->player, 0.0f, -turn_speed);
        if (terminal_key_held('d')) player_rotate(game->player, 0.0f, turn_speed);

        // Jump and fly
        if (terminal_key_pressed(' ')) player_jump(game->player);
        if (terminal_key_pressed('f')) game->player->flying = !game->player->flying;

        // Block interaction
        if (terminal_key_pressed('e')) player_place_block(game->player, game->world, game->player->selected_slot);
        if (terminal_key_pressed('r')) player_break_block(game->player, game->world);

        // Select inventory slot with number keys
        for (int i = 1; i <= 9; i++) {
            char key = '0' + i;
            if (terminal_key_pressed(key)) {
                player_select_slot(game->player, i);
            }
        }

        // Toggle HUD
        if (terminal_key_pressed('h')) renderer_toggle_hud(game->renderer);

        // Toggle debug info
        if (terminal_key_pressed('o')) renderer_toggle_debug(game->renderer);

        // Toggle minimap
        if (terminal_key_pressed('m')) renderer_toggle_minimap(game->renderer);
    }
}

// Handle terminal resize
void handle_resize(GameState* game) {
    if (!game) return;

    // Get new terminal size
    int width, height;
    terminal_get_size(&width, &height);

    // Recreate renderer
    renderer_destroy(game->renderer);
    game->renderer = renderer_create(width, height);
}

// Show title screen
void show_title_screen(GameState* game) {
    if (!game) return;

    // Clear screen
    terminal_clear();

    // Get screen dimensions
    int width, height;
    terminal_get_size(&width, &height);

    // Title
    const char* title = "VOXEL EXPLORER";
    int title_len = strlen(title);
    int title_x = (width - title_len) / 2;
    int title_y = height / 4;

    terminal_draw_colored_string(title_x, title_y, title, COLOR_WHITE | COLOR_BRIGHT, COLOR_BLACK);

    // Subtitle
    const char* subtitle = "An ASCII Raycasting Engine";
    int subtitle_len = strlen(subtitle);
    int subtitle_x = (width - subtitle_len) / 2;
    int subtitle_y = title_y + 2;

    terminal_draw_colored_string(subtitle_x, subtitle_y, subtitle, COLOR_WHITE, COLOR_BLACK);

    // Instructions
    const char* instructions[] = {
        "Controls:",
        "WASD - Look around",
        "IJKL - Move",
        "Space - Jump",
        "F - Toggle flying",
        "E - Place block",
        "R - Break block",
        "1-9 - Select block type",
        "P - Pause game",
        "Q - Quit",
        "",
        "Press any key to start..."
    };

    int num_instructions = sizeof(instructions) / sizeof(instructions[0]);
    int instructions_y = height / 2;

    for (int i = 0; i < num_instructions; i++) {
        int len = strlen(instructions[i]);
        int x = (width - len) / 2;
        int y = instructions_y + i;

        terminal_draw_colored_string(x, y, instructions[i], COLOR_WHITE, COLOR_BLACK);
    }

    // Present screen
    terminal_flush();

    // Wait for keypress
    while (1) {
        terminal_process_input();

        for (int i = 0; i < 256; i++) {
            if (terminal_key_pressed(i)) {
                return;
            }
        }

        terminal_sleep(50);
    }
}

// Show pause menu
void show_pause_menu(GameState* game) {
    if (!game) return;

    // Get screen dimensions
    int width = game->renderer->width;
    int height = game->renderer->height;

    // Draw pause menu background
    for (int y = height / 4; y < height * 3 / 4; y++) {
        for (int x = width / 4; x < width * 3 / 4; x++) {
            renderer_set_pixel(game->renderer, x, y, ' ', COLOR_BLACK, COLOR_BLUE);
        }
    }

    // Draw border
    for (int y = height / 4; y < height * 3 / 4; y++) {
        renderer_set_pixel(game->renderer, width / 4, y, '#', COLOR_WHITE, COLOR_BLUE);
        renderer_set_pixel(game->renderer, width * 3 / 4 - 1, y, '#', COLOR_WHITE, COLOR_BLUE);
    }

    for (int x = width / 4; x < width * 3 / 4; x++) {
        renderer_set_pixel(game->renderer, x, height / 4, '#', COLOR_WHITE, COLOR_BLUE);
        renderer_set_pixel(game->renderer, x, height * 3 / 4 - 1, '#', COLOR_WHITE, COLOR_BLUE);
    }

    // Draw title
    const char* title = "GAME PAUSED";
    int title_len = strlen(title);
    int title_x = (width - title_len) / 2;
    int title_y = height / 3;

    renderer_draw_text(game->renderer, title_x, title_y, title, COLOR_WHITE | COLOR_BRIGHT, COLOR_BLUE);

    // Draw menu options
    const char* options[] = {
        "R - Resume Game",
        "Q - Quit Game"
    };

    int num_options = sizeof(options) / sizeof(options[0]);
    int options_y = height / 2;

    for (int i = 0; i < num_options; i++) {
        int len = strlen(options[i]);
        int x = (width - len) / 2;
        int y = options_y + i;

        renderer_draw_text(game->renderer, x, y, options[i], COLOR_WHITE, COLOR_BLUE);
    }

    // Draw controls reminder
    const char* controls = "WASD: Look | IJKL: Move | Space: Jump | F: Fly | E: Place | R: Break";
    int controls_len = strlen(controls);
    int controls_x = (width - controls_len) / 2;
    int controls_y = height * 2 / 3;

    renderer_draw_text(game->renderer, controls_x, controls_y, controls, COLOR_WHITE, COLOR_BLUE);
}
