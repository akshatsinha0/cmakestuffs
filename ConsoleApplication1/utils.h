/**
 * @file utils.h
 * @brief Utility functions and helpers
 */
#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

 // Logging levels
#define LOG_DEBUG 0
#define LOG_INFO 1
#define LOG_WARNING 2
#define LOG_ERROR 3

// Time utilities
unsigned long long get_time_ms(void);
void set_random_seed(unsigned int seed);
int random_int(int min, int max);
float random_float(float min, float max);

// Noise functions for terrain generation
float noise2d(float x, float y, int seed);
float noise3d(float x, float y, float z, int seed);
float perlin_noise2d(float x, float y, int octaves, float persistence, int seed);
float perlin_noise3d(float x, float y, float z, int octaves, float persistence, int seed);

// Math utilities
float clamp(float value, float min, float max);
float lerp(float a, float b, float t);
float smoothstep(float edge0, float edge1, float x);
int max_int(int a, int b);
int min_int(int a, int b);
float max_float(float a, float b);
float min_float(float a, float b);

// Logging functions
void log_message(int level, const char* format, ...);
void set_log_level(int level);
void set_log_file(FILE* file);

// String utilities
char* str_duplicate(const char* str);
char* str_concat(const char* str1, const char* str2);
int str_ends_with(const char* str, const char* suffix);
char* str_trim(char* str);

#endif /* UTILS_H */
