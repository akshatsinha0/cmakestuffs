/**
 * @file terminal.c
 * @brief Implementation of terminal functions
 */
#define _CRT_SECURE_NO_WARNINGS

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4996) // Disable sprintf warnings
#endif

#include "terminal.h"
#include "config.h"
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <time.h>
#endif

 // Global state
static int terminal_width = 0;
static int terminal_height = 0;
static char keystate[256] = { 0 };
static char keyheld[256] = { 0 };

#ifdef _WIN32
static HANDLE hConsole = NULL;
static DWORD dwOriginalMode = 0;
#else
static struct termios original_termios;
#endif

// Initialize the terminal
int terminal_init(void) {
#ifdef _WIN32
    // Get console handle
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) {
        return 0;
    }

    // Get original console mode
    if (!GetConsoleMode(hConsole, &dwOriginalMode)) {
        return 0;
    }

    // Set new console mode
    DWORD dwNewMode = dwOriginalMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(hConsole, dwNewMode)) {
        return 0;
    }

    // Get console screen buffer info
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        return 0;
    }

    // Set terminal size
    terminal_width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    terminal_height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

    // Hide cursor
    CONSOLE_CURSOR_INFO cursor_info;
    cursor_info.dwSize = 1;
    cursor_info.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &cursor_info);

    // Clear screen
    system("cls");
#else
    // Get terminal attributes
    if (tcgetattr(STDIN_FILENO, &original_termios) == -1) {
        return 0;
    }

    // Set new terminal attributes
    struct termios new_termios = original_termios;
    new_termios.c_lflag &= ~(ICANON | ECHO);
    if (tcsetattr(STDIN_FILENO, TCSANOW, &new_termios) == -1) {
        return 0;
    }

    // Set non-blocking input
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

    // Get terminal size
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) {
        terminal_width = 80;
        terminal_height = 24;
    }
    else {
        terminal_width = ws.ws_col;
        terminal_height = ws.ws_row;
    }

    // Hide cursor and clear screen
    printf("\033[?25l");
    printf("\033[2J");
#endif

    // Clear key states
    memset(keystate, 0, sizeof(keystate));
    memset(keyheld, 0, sizeof(keyheld));

    return 1;
}

// Cleanup terminal
void terminal_cleanup(void) {
#ifdef _WIN32
    // Restore original console mode
    if (hConsole != NULL && hConsole != INVALID_HANDLE_VALUE) {
        SetConsoleMode(hConsole, dwOriginalMode);

        // Show cursor
        CONSOLE_CURSOR_INFO cursor_info;
        cursor_info.dwSize = 1;
        cursor_info.bVisible = TRUE;
        SetConsoleCursorInfo(hConsole, &cursor_info);
    }
#else
    // Restore terminal attributes
    tcsetattr(STDIN_FILENO, TCSANOW, &original_termios);

    // Show cursor
    printf("\033[?25h");
#endif

    // Reset colors and clear screen
    terminal_reset_color();
    terminal_clear();
    terminal_set_cursor(0, 0);
}

// Process input
void terminal_process_input(void) {
    // Reset key states (but maintain held states)
    for (int i = 0; i < 256; i++) {
        keyheld[i] = keystate[i] || keyheld[i];
        keystate[i] = 0;
    }

#ifdef _WIN32
    // Check for key presses using _kbhit and _getch
    while (_kbhit()) {
        int ch = _getch();

        // Handle special keys (arrow keys, etc.)
        if (ch == 0 || ch == 0xE0) {
            ch = _getch();

            // Map extended keys to values within 0-255 range
            switch (ch) {
            case 72: if (KEY_UP < 256) keystate[KEY_UP] = 1; break;
            case 80: if (KEY_DOWN < 256) keystate[KEY_DOWN] = 1; break;
            case 75: if (KEY_LEFT < 256) keystate[KEY_LEFT] = 1; break;
            case 77: if (KEY_RIGHT < 256) keystate[KEY_RIGHT] = 1; break;
            case 71: if (KEY_HOME < 256) keystate[KEY_HOME] = 1; break;
            case 79: if (KEY_END < 256) keystate[KEY_END] = 1; break;
            case 73: if (KEY_PGUP < 256) keystate[KEY_PGUP] = 1; break;
            case 81: if (KEY_PGDN < 256) keystate[KEY_PGDN] = 1; break;
            case 82: if (KEY_INSERT < 256) keystate[KEY_INSERT] = 1; break;
            case 83: if (KEY_DELETE < 256) keystate[KEY_DELETE] = 1; break;
            case 59: if (KEY_F1 < 256) keystate[KEY_F1] = 1; break;
            case 60: if (KEY_F2 < 256) keystate[KEY_F2] = 1; break;
            case 61: if (KEY_F3 < 256) keystate[KEY_F3] = 1; break;
            case 62: if (KEY_F4 < 256) keystate[KEY_F4] = 1; break;
            case 63: if (KEY_F5 < 256) keystate[KEY_F5] = 1; break;
            case 64: if (KEY_F6 < 256) keystate[KEY_F6] = 1; break;
            case 65: if (KEY_F7 < 256) keystate[KEY_F7] = 1; break;
            case 66: if (KEY_F8 < 256) keystate[KEY_F8] = 1; break;
            case 67: if (KEY_F9 < 256) keystate[KEY_F9] = 1; break;
            case 68: if (KEY_F10 < 256) keystate[KEY_F10] = 1; break;
            case 133: if (KEY_F11 < 256) keystate[KEY_F11] = 1; break;
            case 134: if (KEY_F12 < 256) keystate[KEY_F12] = 1; break;
            }
        }
        else {
            // Map normal keys and special keys
            switch (ch) {
            case 8: if (KEY_BACKSPACE < 256) keystate[KEY_BACKSPACE] = 1; break;
            case 9: if (KEY_TAB < 256) keystate[KEY_TAB] = 1; break;
            case 13: if (KEY_ENTER < 256) keystate[KEY_ENTER] = 1; break;
            case 27: if (KEY_ESCAPE < 256) keystate[KEY_ESCAPE] = 1; break;
            case 32: if (KEY_SPACE < 256) keystate[KEY_SPACE] = 1; break;
            default: keystate[ch & 0xFF] = 1; break;
            }
        }
    }
#else
    // Check for key presses using read
    char c;
    while (read(STDIN_FILENO, &c, 1) > 0) {
        // Handle escape sequences
        if (c == 27) {
            char seq[4] = { 0 };
            if (read(STDIN_FILENO, &seq[0], 1) <= 0) {
                if (KEY_ESCAPE < 256) keystate[KEY_ESCAPE] = 1;
                continue;
            }

            if (seq[0] == '[') {
                if (read(STDIN_FILENO, &seq[1], 1) <= 0) {
                    continue;
                }

                if (seq[1] >= '0' && seq[1] <= '9') {
                    if (read(STDIN_FILENO, &seq[2], 1) <= 0) {
                        continue;
                    }

                    if (seq[2] == '~') {
                        switch (seq[1]) {
                        case '1': if (KEY_HOME < 256) keystate[KEY_HOME] = 1; break;
                        case '2': if (KEY_INSERT < 256) keystate[KEY_INSERT] = 1; break;
                        case '3': if (KEY_DELETE < 256) keystate[KEY_DELETE] = 1; break;
                        case '4': if (KEY_END < 256) keystate[KEY_END] = 1; break;
                        case '5': if (KEY_PGUP < 256) keystate[KEY_PGUP] = 1; break;
                        case '6': if (KEY_PGDN < 256) keystate[KEY_PGDN] = 1; break;
                        }
                    }
                }
                else {
                    switch (seq[1]) {
                    case 'A': if (KEY_UP < 256) keystate[KEY_UP] = 1; break;
                    case 'B': if (KEY_DOWN < 256) keystate[KEY_DOWN] = 1; break;
                    case 'C': if (KEY_RIGHT < 256) keystate[KEY_RIGHT] = 1; break;
                    case 'D': if (KEY_LEFT < 256) keystate[KEY_LEFT] = 1; break;
                    case 'H': if (KEY_HOME < 256) keystate[KEY_HOME] = 1; break;
                    case 'F': if (KEY_END < 256) keystate[KEY_END] = 1; break;
                    }
                }
            }
            else if (seq[0] == 'O') {
                if (read(STDIN_FILENO, &seq[1], 1) <= 0) {
                    continue;
                }

                switch (seq[1]) {
                case 'P': if (KEY_F1 < 256) keystate[KEY_F1] = 1; break;
                case 'Q': if (KEY_F2 < 256) keystate[KEY_F2] = 1; break;
                case 'R': if (KEY_F3 < 256) keystate[KEY_F3] = 1; break;
                case 'S': if (KEY_F4 < 256) keystate[KEY_F4] = 1; break;
                }
            }
        }
        else {
            // Map normal keys
            switch (c) {
            case 127: if (KEY_BACKSPACE < 256) keystate[KEY_BACKSPACE] = 1; break;
            case 9: if (KEY_TAB < 256) keystate[KEY_TAB] = 1; break;
            case 10: if (KEY_ENTER < 256) keystate[KEY_ENTER] = 1; break;
            case 27: if (KEY_ESCAPE < 256) keystate[KEY_ESCAPE] = 1; break;
            case 32: if (KEY_SPACE < 256) keystate[KEY_SPACE] = 1; break;
            default: keystate[(unsigned char)c] = 1; break;
            }
        }
    }
#endif
}

// Check if a key was pressed
int terminal_key_pressed(char key) {
    return keystate[(unsigned char)key];
}

// Check if a key is held down
int terminal_key_held(char key) {
    return keyheld[(unsigned char)key];
}

// Check if a special key was pressed
int terminal_special_key_pressed(int key) {
    if (key >= 0 && key < 256) {
        return keystate[key];
    }
    return 0;
}

// Clear the terminal
void terminal_clear(void) {
#ifdef _WIN32
    // Clear screen
    system("cls");
#else
    // Clear screen
    printf("\033[2J");
#endif
}

// Set cursor position
void terminal_set_cursor(int x, int y) {
    printf("\033[%d;%dH", y + 1, x + 1);
}

// Set color
void terminal_set_color(int fg, int bg) {
    int bright_fg = (fg & COLOR_BRIGHT) ? 1 : 0;
    int bright_bg = (bg & COLOR_BRIGHT) ? 1 : 0;

    fg &= ~COLOR_BRIGHT;
    bg &= ~COLOR_BRIGHT;

    printf("\033[%d;%d;%d;%dm",
        bright_fg ? 1 : 0,         // bright foreground
        30 + fg,                    // foreground color
        bright_bg ? 5 : 0,          // bright background
        40 + bg);                   // background color
}

// Reset color
void terminal_reset_color(void) {
    printf("\033[0m");
}

// Put a character at a position
void terminal_put_char(int x, int y, char c) {
    terminal_set_cursor(x, y);
    putchar(c);
}

// Put a colored character at a position
void terminal_put_colored_char(int x, int y, char c, int fg, int bg) {
    terminal_set_cursor(x, y);
    terminal_set_color(fg, bg);
    putchar(c);
    terminal_reset_color();
}

// Flush the terminal
void terminal_flush(void) {
    fflush(stdout);
}

// Draw a string at a position
void terminal_draw_string(int x, int y, const char* str) {
    if (!str) return;
    terminal_set_cursor(x, y);
    printf("%s", str);
}

// Draw a colored string at a position
void terminal_draw_colored_string(int x, int y, const char* str, int fg, int bg) {
    if (!str) return;
    terminal_set_cursor(x, y);
    terminal_set_color(fg, bg);
    printf("%s", str);
    terminal_reset_color();
}

// Get terminal size
void terminal_get_size(int* width, int* height) {
    if (width) *width = terminal_width;
    if (height) *height = terminal_height;
}

// Sleep for specified milliseconds
void terminal_sleep(int ms) {
#ifdef _WIN32
    Sleep(ms);
#else
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    nanosleep(&ts, NULL);
#endif
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
