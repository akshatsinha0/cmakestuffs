/**
 * @file terminal.h
 * @brief Terminal handling for input and display
 */
#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdint.h>

 // Terminal color codes
#define COLOR_BLACK 0
#define COLOR_RED 1
#define COLOR_GREEN 2
#define COLOR_YELLOW 3
#define COLOR_BLUE 4
#define COLOR_MAGENTA 5
#define COLOR_CYAN 6
#define COLOR_WHITE 7
#define COLOR_BRIGHT 8

// Terminal initialization and cleanup
int terminal_init(void);
void terminal_cleanup(void);

// Input handling
void terminal_process_input(void);
int terminal_key_pressed(char key);
int terminal_key_held(char key);
int terminal_special_key_pressed(int key);

// Display functions
void terminal_clear(void);
void terminal_set_cursor(int x, int y);
void terminal_set_color(int fg, int bg);
void terminal_reset_color(void);
void terminal_put_char(int x, int y, char c);
void terminal_put_colored_char(int x, int y, char c, int fg, int bg);
void terminal_flush(void);
void terminal_draw_string(int x, int y, const char* str);
void terminal_draw_colored_string(int x, int y, const char* str, int fg, int bg);
void terminal_get_size(int* width, int* height);

// Sleep for specified milliseconds
void terminal_sleep(int ms);

// Special keys
enum SpecialKeys {
    KEY_UP = 256,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_ENTER,
    KEY_ESCAPE,
    KEY_SPACE,
    KEY_BACKSPACE,
    KEY_TAB,
    KEY_HOME,
    KEY_END,
    KEY_PGUP,
    KEY_PGDN,
    KEY_INSERT,
    KEY_DELETE,
    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_F11,
    KEY_F12
};

#endif /* TERMINAL_H */
