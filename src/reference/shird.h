#ifndef SHIRD_H
#define SHIRD_H

#include "../include/SDL2/SDL.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "settings.h"

////////////////////////
//        UTIL
////////////////////////

//util macros
#define ERR_EXT(...) {printf(__VA_ARGS__); exit(1);}
#define ERR_RET(R, ...) {return R; printf(__VA_ARGS__);}

//types
typedef uint8_t   u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t    i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef float    f32;
typedef double   f64;

//vectors
typedef f32 vec2f[2];
typedef i32 vec2i[2];
typedef u32 vec2u[2];

#define VEC2I_TOF(A) {(f32)A[0], (f32)A[1]}
#define VEC2F_TOI(A) {(i32)A[0], (i32)A[1]}
#define VEC2_VOID(A) A[0] = 0; A[1] = 0

//colors
#define BLACK   0xFF000000
#define WHITE   0xFFFFFFFF
#define RED     0xFFFF0000
#define BLUE    0xFF0000FF
#define GREEN   0xFF00FF00
#define MAGENTA 0xFFFF00FF
#define YELLOW  0xFFFFFF00
#define CYAN    0xFF00FFFF
#define MASK    0x00000000

//array list
typedef struct {
	void *items;
	size_t capacity;
	size_t len;
	size_t item_size;
} Array_List;

Array_List *array_list_create(size_t item_size, size_t initial_capacity);
size_t array_list_append(Array_List *arr, void *item);
void *array_list_get(Array_List *arr, size_t index);
u8 array_list_remove(Array_List *arr, size_t index);


////////////////////////
//      SPRITE
////////////////////////

typedef struct {
	u32 *data;
	u32 w;
	u32 h;
} Shird_Sprite;

Shird_Sprite shird_sprite_load(char *path);
void shird_sprite_free(Shird_Sprite *spr);
Shird_Sprite shird_sprite_crop(Shird_Sprite *spr, vec2u pos, vec2u size);
void shird_sprite_flip(Shird_Sprite *spr, bool x, bool y);

Array_List *shird_spritesheet_load(char *path, vec2u size);
void shird_spritesheet_free(Array_List *spr);

typedef struct {
	Array_List *sprites;
	u32 index;
	f32 delay;
	f32 delay_counter;
	bool repeat;
	bool ended;
} Shird_Animation;

Shird_Animation shird_animation_create(char* path, vec2u size, f32 delay);
void shird_animation_flip(Shird_Animation *ani, bool x, bool y);
void shird_animation_free(Shird_Animation *ani);

////////////////////////
//       FONT
////////////////////////

#define SHIRD_FONT_LEN 68

typedef struct {
	Array_List *sprites;
	vec2u char_size;
} Shird_Font;

Shird_Font shird_font_create(char *path);

////////////////////////
//      RENDER
////////////////////////
extern SDL_Window *shird_window;
extern bool shird_should_quit;
extern SDL_Renderer *shird_renderer;
extern SDL_Texture *shird_texture;
extern u32 shird_buffer[SHIRD_BUFFER_WIDTH*SHIRD_BUFFER_HEIGHT];

void shird_render_init(void);
void shird_render_update_buffer(void);
void shird_render_free(void);

void shird_render_clear(u32 color);
void shird_render_pixel(i32 x, i32 y, u32 color);
void shird_render_rect(vec2i pos, vec2u size, u32 color);

void shird_render_sprite(Shird_Sprite spr, i32 x, i32 y);
void shird_render_font(Shird_Font *font, i32 x, i32 y, char *text, u32 color);
void shird_render_animation(Shird_Animation *animation, i32 x, i32 y);

////////////////////////
//        TIME
////////////////////////
extern f32 shird_time_delta;
extern f32 shird_time_now;
extern f32 shird_time_last;
extern f32 shird_time_frame_last;
extern f32 shird_time_frame_delay;
extern f32 shird_time_frame_time;
extern u32 shird_time_frame_rate;
extern u32 shird_time_frame_count;

void shird_time_init(u32 frame_rate);
void shird_time_update(void);
void shird_time_update_late(void);

////////////////////////
//       INPUT
////////////////////////

typedef enum {
	SHIRD_KEY_UP,
	SHIRD_KEY_DOWN,
	SHIRD_KEY_UNKNOWN
} Shird_Key_Status;

Shird_Key_Status shird_input_get(const char *name);

typedef enum {
	SHIRD_MOUSE_DOWN,
	SHIRD_MOUSE_UP,
	SHIRD_MOUSE_UNKNOWN
} Shird_Mouse_Status;

Shird_Mouse_Status shird_input_mouse_get(const char *button_name);
void shird_input_mouse(vec2i *vec);

////////////////////////
//      PHYSICS
////////////////////////

typedef enum {
	SHIRD_DIRECTION_RIGHT,
	SHIRD_DIRECTION_LEFT,
	SHIRD_DIRECTION_UP,
	SHIRD_DIRECTION_DOWN,
	SHIRD_DIRECTION_NULL
} Shird_Direction;

typedef struct {
	vec2f pos;
	vec2u size;
} Shird_AABB;

bool shird_physics_intersect(Shird_AABB a, Shird_AABB b);
Shird_Direction shird_physics_collision(Shird_AABB a, Shird_AABB b);

////////////////////////
//        IO
////////////////////////

#define SHIRD_IO_READ_CHUNK_SIZE 2097152
#define SHIRD_IO_READ_ERROR_GENERAL "error reading file: %s, errno %d\n"
#define SHIRD_IO_READ_ERROR_MEMORY "not enough memory to read the file: %s\n"

typedef struct {
	char *data;
	size_t len;
	u8 is_valid;
} Shird_File;

Shird_File shird_io_file_read(const char *path);
u8 shird_io_file_write(void *buffer, size_t size, const char *path);

#endif
