//// minecraft.c - Windows-compatible ASCII Raycaster
//#define _CRT_SECURE_NO_WARNINGS
//
//#ifdef _WIN32
//#include <windows.h>
//#include <conio.h>
//#else
//#include <unistd.h>
//#include <fcntl.h>
//#include <termios.h>
//#endif
//
//#include <stdio.h>
//#include <stdlib.h>
//#include <math.h>
//#include <stdint.h>
//
//// Configuration
//#define Y_PIXELS 40
//#define X_PIXELS 120
//#define Z_BLOCKS 10
//#define Y_BLOCKS 20
//#define X_BLOCKS 20
//#define EYE_HEIGHT 1.5f
//#define VIEW_HEIGHT 0.7f
//#define VIEW_WIDTH 1.0f
//#define BLOCK_BORDER_SIZE 0.05f
//
//// Vector structures
//typedef struct { float x, y, z; } Vector3;
//typedef struct { float psi, phi; } Vector2;
//typedef struct { Vector3 pos; Vector2 view; } Player;
//
//// Function prototypes
//void init_terminal(void);
//void restore_terminal(void);
//void process_input(void);
//Vector3 angles_to_dir(Vector2 angles);
//char raycast(Vector3 pos, Vector3 dir);
//void render_frame(void);
//
//// Global state
//static char*** blocks;
//static char** frame_buffer;
//static Player player;
//static char keystate[256];
//
//#ifdef _WIN32
//static HANDLE hConsole;
//static DWORD dwOriginalMode;
//#else
//static struct termios old_term, new_term;
//#endif
//
//void init_terminal() {
//#ifdef _WIN32
//    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
//    GetConsoleMode(hConsole, &dwOriginalMode);
//    SetConsoleMode(hConsole, dwOriginalMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
//#else
//    tcgetattr(STDIN_FILENO, &old_term);
//    new_term = old_term;
//    new_term.c_lflag &= ~(ICANON | ECHO);
//    tcsetattr(STDIN_FILENO, TCSANOW, &new_term);
//    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
//#endif
//}
//
//void restore_terminal() {
//#ifdef _WIN32
//    SetConsoleMode(hConsole, dwOriginalMode);
//#else
//    tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
//#endif
//}
//
//void process_input() {
//#ifdef _WIN32
//    for (int i = 0; i < 256; i++) keystate[i] = 0;
//    while (_kbhit()) {
//        int ch = _getch();
//        if (ch == 0 || ch == 0xE0) ch = _getch() + 0x100;
//        keystate[ch & 0xFF] = 1;
//    }
//#else
//    char c;
//    while (read(STDIN_FILENO, &c, 1) > 0)
//        keystate[(unsigned char)c] = 1;
//#endif
//}
//
//Vector3 angles_to_dir(Vector2 angles) {
//    Vector3 result;
//    result.x = cosf(angles.psi) * cosf(angles.phi);
//    result.y = cosf(angles.psi) * sinf(angles.phi);
//    result.z = sinf(angles.psi);
//    return result;
//}
//
//char raycast(Vector3 pos, Vector3 dir) {
//    float eps = 0.01f;
//    while (1) {
//        if (pos.x < 0 || pos.x >= X_BLOCKS ||
//            pos.y < 0 || pos.y >= Y_BLOCKS ||
//            pos.z < 0 || pos.z >= Z_BLOCKS) return ' ';
//
//        int x = (int)pos.x, y = (int)pos.y, z = (int)pos.z;
//        if (blocks[z][y][x] != ' ') {
//            int borders = 0;
//            if (fabsf(pos.x - x - 0.5f) < BLOCK_BORDER_SIZE) borders++;
//            if (fabsf(pos.y - y - 0.5f) < BLOCK_BORDER_SIZE) borders++;
//            if (fabsf(pos.z - z - 0.5f) < BLOCK_BORDER_SIZE) borders++;
//            return borders >= 2 ? '#' : blocks[z][y][x];
//        }
//
//        float t = INFINITY;
//        if (dir.x > eps) t = fminf(t, (floorf(pos.x + 1) - pos.x) / dir.x);
//        if (dir.x < -eps) t = fminf(t, (ceilf(pos.x - 1) - pos.x) / dir.x);
//        if (dir.y > eps) t = fminf(t, (floorf(pos.y + 1) - pos.y) / dir.y);
//        if (dir.y < -eps) t = fminf(t, (ceilf(pos.y - 1) - pos.y) / dir.y);
//        if (dir.z > eps) t = fminf(t, (floorf(pos.z + 1) - pos.z) / dir.z);
//        if (dir.z < -eps) t = fminf(t, (ceilf(pos.z - 1) - pos.z) / dir.z);
//
//        pos.x += dir.x * (t + eps);
//        pos.y += dir.y * (t + eps);
//        pos.z += dir.z * (t + eps);
//    }
//}
//
//void render_frame() {
//    Vector2 view = player.view;
//    Vector3 dir = angles_to_dir(view);
//
//    for (int y = 0; y < Y_PIXELS; y++) {
//        float vert_angle = VIEW_HEIGHT * (0.5f - (float)y / Y_PIXELS);
//        for (int x = 0; x < X_PIXELS; x++) {
//            float horiz_angle = VIEW_WIDTH * ((float)x / X_PIXELS - 0.5f);
//            Vector2 ray_angles = { view.psi + vert_angle, view.phi + horiz_angle };
//            Vector3 ray_dir = angles_to_dir(ray_angles);
//            frame_buffer[y][x] = raycast(player.pos, ray_dir);
//        }
//    }
//}
//
//int main() {
//    init_terminal();
//
//    // Initialize world
//    blocks = (char***)malloc(Z_BLOCKS * sizeof(char**));
//    for (int z = 0; z < Z_BLOCKS; z++) {
//        blocks[z] = (char**)malloc(Y_BLOCKS * sizeof(char*));
//        for (int y = 0; y < Y_BLOCKS; y++) {
//            blocks[z][y] = (char*)malloc(X_BLOCKS * sizeof(char));
//            for (int x = 0; x < X_BLOCKS; x++) {
//                blocks[z][y][x] = (z < 2) ? '#' : ' ';
//            }
//        }
//    }
//
//    // Initialize player
//    player.pos.x = X_BLOCKS / 2.0f;
//    player.pos.y = Y_BLOCKS / 2.0f;
//    player.pos.z = EYE_HEIGHT + 2;
//    player.view.psi = 0;
//    player.view.phi = 0;
//
//    // Allocate frame buffer
//    frame_buffer = (char**)malloc(Y_PIXELS * sizeof(char*));
//    for (int y = 0; y < Y_PIXELS; y++) {
//        frame_buffer[y] = (char*)malloc(X_PIXELS * sizeof(char));
//    }
//
//    // Main loop
//    while (1) {
//        process_input();
//        if (keystate['q']) break;
//
//        // Movement
//        Vector3 move_dir = angles_to_dir(player.view);
//        float speed = 0.2f;
//
//        if (keystate['w']) player.view.psi += 0.03f;
//        if (keystate['s']) player.view.psi -= 0.03f;
//        if (keystate['a']) player.view.phi -= 0.03f;
//        if (keystate['d']) player.view.phi += 0.03f;
//
//        if (keystate['i']) {
//            player.pos.x += move_dir.x * speed;
//            player.pos.y += move_dir.y * speed;
//        }
//        if (keystate['k']) {
//            player.pos.x -= move_dir.x * speed;
//            player.pos.y -= move_dir.y * speed;
//        }
//
//        render_frame();
//
//        // Draw
//        printf("\033[0;0H");
//        for (int y = 0; y < Y_PIXELS; y++) {
//            for (int x = 0; x < X_PIXELS; x++) {
//                putchar(frame_buffer[y][x]);
//            }
//            putchar('\n');
//        }
//
//#ifdef _WIN32
//        Sleep(50);
//#else
//        usleep(50000);
//#endif
//    }
//
//    restore_terminal();
//    return 0;
//}
