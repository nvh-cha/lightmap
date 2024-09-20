#ifndef ENGINE_H
#define ENGINE_H

#include "../include/raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <inttypes.h>
#include <string.h>

// macros
#define ERR_RET(R, ...) { printf(__VA_ARGS__); return R; }
#define ERR_EXT(...) { printf(__VA_ARGS__); exit(1); }
#define IO_READ_CHUNK_SIZE 2097152
#define IO_READ_ERROR_GENERAL "error reading file: %s, errno: %d\n"
#define IO_READ_ERROR_MEMORY "not enough memory to read file: %s\n"

// types
typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;
typedef int64_t  i64;
typedef int32_t  i32;
typedef int16_t  i16;
typedef int8_t   i8;
typedef float    f32;
typedef double   f64;
typedef Vector2 vec2;
typedef struct {
  vec2 start;
  vec2 end;
} Line;
f32 distance(vec2 a, vec2 b);

// arraylist
typedef struct {
  void *items;
  size_t item_size;
  u32 len;
  size_t capacity;
} ArrayList;
ArrayList arraylist_create(size_t item_size, size_t initial_capacity);
bool arraylist_append(ArrayList *arr, void *item);
void *arraylist_get(ArrayList *arr, size_t i);
bool arraylist_remove(ArrayList *arr, size_t i);

// spritesheet
typedef struct {
  Texture2D *sprites;
  u32 len;
} Spritesheet;

Spritesheet spritesheet_create(vec2 size, char *path);
Texture2D spritesheet_get(Spritesheet s, u32 i);
void spritesheet_free(Spritesheet *s);

// file io
typedef struct {
	char *data;
	size_t len;
	u8 is_valid;
} File;

File io_file_read(const char *path);
u8 io_file_write(void *buffer, size_t size, const char *path);

// light
typedef struct {
  vec2 pos;
  f32 radius;
  Color color;
} Light;

void light_render(Light *l); // has to be inside texturemode
void light_shadow(Light l, Line line, Color color);
void light_shadow_rect(Light l, Rectangle rect, Color color);

void lightmap_render(RenderTexture *lightmap);
#endif
