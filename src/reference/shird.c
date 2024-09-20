#include "shird.h"
#include "settings.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//static stuff
static char characters[SHIRD_FONT_LEN] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 
    'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 
    'u', 'v', 'w', 'x', 'y', 'z', 
    ' ', '1', '2', '3', '4', '5', '6', '7', '8', '9', 
    '0', ':', ';', '-', '.', ','
};

static u32 get_char_index(char c) {
	for (u32 i=0;i<SHIRD_FONT_LEN;i++) {
		if (characters[i] == c)
			return i;
	}
	ERR_RET(-1, "get_char_index failed: %c\n", c);
}

////////////////////////
//        UTIL
////////////////////////

Array_List *array_list_create(size_t item_size, size_t initial_capacity) {
	Array_List *arr = malloc(sizeof(Array_List));
	if (!arr)
		ERR_RET(NULL, "failed to allocate memory for the array_list\n");
	arr->item_size = item_size;
	arr->capacity = initial_capacity;
	arr->len = 0;
	arr->items = malloc(item_size*arr->capacity);
	if (!arr->items)
		ERR_RET(NULL, "failed to allocate memory for array_list\n");
	return arr;
}

size_t array_list_append(Array_List *arr, void *item) {
	if (arr->capacity == arr->len) {
		arr->capacity = arr->capacity > 0 ? arr->capacity*2 : 1;
		void *items = realloc(arr->items, arr->item_size*arr->capacity);
		if (!items) ERR_RET(-1, "failed to realloc memory for array_list\n");
		arr->items = items;
	}
	size_t index = arr->len++;
	memcpy(arr->items+arr->item_size*index, item, arr->item_size);
	return index;
}

void *array_list_get(Array_List *arr, size_t index) {
	if (index >= arr->len)
		ERR_RET(NULL, "index out of bounds\n");
	return arr->items+index*arr->item_size;
}

u8 array_list_remove(Array_List *arr, size_t index) {
	if (arr->len == 0) ERR_RET(1, "list is empty\n");
	if (index >= arr->len) ERR_RET(1, "index out of bounds\n");

	if (arr->len == 1) {
		arr->len = 0;
		return 0;
	}

	--arr->len;

	u8 *item_ptr = (u8*)arr->items + index * arr->item_size;
	u8 *end_ptr = (u8*)arr->items + arr->len * arr->item_size;
	memcpy(item_ptr, end_ptr, arr->item_size);

	return 0;
}

////////////////////////
//      RENDER
////////////////////////

SDL_Window *shird_window = NULL;
SDL_Renderer *shird_renderer = NULL;
SDL_Texture *shird_texture = NULL;
u32 shird_buffer[SHIRD_BUFFER_WIDTH*SHIRD_BUFFER_HEIGHT];
bool shird_should_quit = false;

void shird_render_init(void) {
    shird_window = SDL_CreateWindow(
        SHIRD_TITLE,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        SHIRD_BUFFER_WIDTH*SHIRD_BUFFER_SCALE,
        SHIRD_BUFFER_HEIGHT*SHIRD_BUFFER_SCALE,
        SDL_WINDOW_SHOWN
    );
    if (!shird_window)
        ERR_EXT("failed to init window: %s\n", SDL_GetError());

    shird_renderer = SDL_CreateRenderer(shird_window, -1, SDL_RENDERER_ACCELERATED);
    if (!shird_renderer) ERR_EXT("failed to init sdl renderer: %s\n", SDL_GetError())
    
    shird_texture = SDL_CreateTexture(shird_renderer, SDL_PIXELFORMAT_ARGB8888,
                                      SDL_TEXTUREACCESS_STREAMING,
                                      SHIRD_BUFFER_WIDTH, SHIRD_BUFFER_HEIGHT);
    if (!shird_texture) ERR_EXT("failed to init texture: %s\n", SDL_GetError())

    for (u32 i=0;i<SHIRD_BUFFER_WIDTH*SHIRD_BUFFER_HEIGHT;i++) {
        shird_buffer[i] = 0xFFFFFFFF;
    }
}

void shird_render_update_buffer(void) {
    SDL_UpdateTexture(shird_texture, NULL, shird_buffer, SHIRD_BUFFER_WIDTH*sizeof(u32));
    SDL_RenderClear(shird_renderer);
    SDL_RenderCopy(shird_renderer, shird_texture, NULL, NULL);
    SDL_RenderPresent(shird_renderer);
}

void shird_render_free(void) {
    SDL_DestroyRenderer(shird_renderer);
    SDL_DestroyTexture(shird_texture);
    SDL_DestroyWindow(shird_window);
}

void shird_render_clear(u32 color) {
    for (u32 i=0;i<SHIRD_BUFFER_WIDTH*SHIRD_BUFFER_HEIGHT;i++) {
        shird_buffer[i] = color;
    }
}

static u32 blend(u32 src, u32 dest) {
    u8 srcA = (src >> 24) & 0xFF;
    u8 srcR = (src >> 16) & 0xFF;
    u8 srcG = (src >> 8) & 0xFF;
    u8 srcB = src & 0xFF;

    u8 destR = (dest >> 16) & 0xFF;
    u8 destG = (dest >> 8) & 0xFF;
    u8 destB = dest & 0xFF;

    f32 alpha = srcA / 255.0f;

    u8 outR = (u8)(srcR * alpha + destR * (1 - alpha));
    u8 outG = (u8)(srcG * alpha + destG * (1 - alpha));
    u8 outB = (u8)(srcB * alpha + destB * (1 - alpha));

    return (0xFF << 24) | (outR << 16) | (outG << 8) | outB;
}

void shird_render_pixel(i32 x, i32 y, u32 color) {
    if (x >= 0 && x < SHIRD_BUFFER_WIDTH && y >= 0 && y < SHIRD_BUFFER_HEIGHT) {
        if (color >> 24 == 0xFF) {
            shird_buffer[x+y*SHIRD_BUFFER_WIDTH] = color;
        } else if (color >> 24 != 0x00) {
            u32 res = blend(color, shird_buffer[x+y*SHIRD_BUFFER_WIDTH]);
            shird_buffer[x+y*SHIRD_BUFFER_WIDTH] = res;
        }
    }
}

void shird_render_rect(vec2i pos, vec2u size, u32 color) {
    for (u32 i=0;i<size[0];i++) {
        for (u32 j=0;j<size[1];j++) {
            shird_render_pixel(pos[0]+i, pos[1]+j, color);
        }
    }
}

void shird_render_sprite(Shird_Sprite spr, i32 x, i32 y) {
    for (u32 i=0;i<spr.w;i++) {
        for (u32 j=0;j<spr.h;j++) {
            shird_render_pixel(x+i, y+j, spr.data[i+j*spr.w]);
        }
    }
}

void shird_render_font(Shird_Font *font, i32 x, i32 y, char *text, u32 color) {
    i32 m = 0;
	for (u32 i=0;i<strlen(text); i++) {
		char c = text[i];
		u32 char_index = get_char_index(c);
		
		Shird_Sprite *spr = array_list_get(font->sprites, char_index);
		//render_sprite(*spr, x+i+m, y);
        for (u32 j=0;j<spr->w;j++) {
            for (u32 h=0;h<spr->h;h++) {
                shird_render_pixel((x+i+m)+j, y+h, (spr->data[j+h*spr->w] >> 24 == 0xFF) ? color : MASK);
            }
        }
		m += spr->w;
	}
}

void shird_render_animation(Shird_Animation *animation, i32 x, i32 y) {
	if (animation->delay_counter > 0) {
		animation->delay_counter -= shird_time_delta;
		shird_render_sprite(*(Shird_Sprite*)array_list_get(animation->sprites, animation->index), x, y);
	} else {
		animation->delay_counter = animation->delay;
		if (animation->repeat) {
			animation->index = (animation->index+1) % animation->sprites->len;
		} else {
			if (!animation->ended) animation->index++;
			if (animation->index == animation->sprites->len-1) animation->ended = true;
		}
	}
}

////////////////////////
//      SPRITE
////////////////////////
Shird_Sprite shird_sprite_load(char *path) {
    i32 w = 0;
    i32 h = 0;
    unsigned char *data = stbi_load(path, &w, &h, &(int){4}, STBI_rgb_alpha);
    if (!data) ERR_EXT("failed to load image: %s\n", path);
    u32 *buffer = (u32*)malloc(w*h*sizeof(u32));
    if (!buffer) {
        stbi_image_free(data);
        ERR_EXT("failed to allocate memory for sprite: %s\n", path);
    }
    for (u32 i=0;i<w*h;++i) {
        u8 a = data[i*4+3];
        u8 r = data[i*4+0];
        u8 g = data[i*4+1];
        u8 b = data[i*4+2];
        buffer[i] = (a << 24) | (r << 16) | (g << 8) | b;
    }
    Shird_Sprite res = (Shird_Sprite) {
        .w = w,
        .h = h,
        .data = buffer
    };
    if (w == 0 && h == 0) ERR_EXT("failed to read image data:%s\n", path);
    return res;
}


Shird_Sprite shird_sprite_crop(Shird_Sprite *sprite, vec2u pos, vec2u size) {
    if (size[0] < 1 || size[1] < 1)
        ERR_EXT("ERROR: (sprite_crop):sprites cant have w or h that are less than 1");
    if (pos[0] > sprite->w || pos[1] > sprite->h)
        ERR_EXT("ERROR: (sprite_crop):out of bounds");
    if (pos[0]+size[0] > sprite->w || pos[1]+size[1] > sprite->h)
        ERR_EXT("ERROR: (sprte_crop):selection out of bounds");

    u32 *pixels = malloc(size[0]*size[1]*sizeof(u32));

    for (int i=0;i<size[1];i++) {
        for (int j=0;j<size[0];j++) {
            pixels[i*size[0]+j] = sprite->data[(u32)((pos[1]+i)*sprite->w+(pos[0]+j))];
        }
    }

    Shird_Sprite out = {
        .data = pixels,
        .w = size[0],
        .h = size[1]
    };
    return out;
}

void shird_sprite_flip(Shird_Sprite *sprite, bool flip_x, bool flip_y) {
    u32 temp;

    if (flip_x) {
        for (int y = 0; y < sprite->h; y++) {
            for (int x = 0; x < sprite->w / 2; x++) {
                int opposite_x = sprite->w - 1 - x;
                temp = sprite->data[y * sprite->w + x];
                sprite->data[y * sprite->w + x] = sprite->data[y * sprite->w + opposite_x];
                sprite->data[y * sprite->w + opposite_x] = temp;
            }
        }
    }

    if (flip_y) {
        for (int y = 0; y < sprite->h / 2; y++) {
            for (int x = 0; x < sprite->w; x++) {
                int opposite_y = sprite->h - 1 - y;
                temp = sprite->data[y * sprite->w + x];
                sprite->data[y * sprite->w + x] = sprite->data[opposite_y * sprite->w + x];
                sprite->data[opposite_y * sprite->w + x] = temp;
            }
        }
    }
}

void shird_sprite_free(Shird_Sprite *spr) {
    free(spr->data);
}

Array_List *shird_spritesheet_load(char *path, vec2u size) {
    Shird_Sprite base = shird_sprite_load(path);
    if (size[0] > base.w || size[1] > base.h || size[0] == 0 || size[1] == 0)
        ERR_RET(NULL, "spritesheet size is too big or 0, sprite: %s\n", path)
    
    u32 len = (base.w / size[0])*(base.h / size[1]);

    Array_List *sprites = array_list_create(sizeof(Shird_Sprite), len);
    for (u32 j=0;j<base.h/size[1];j++) {
        for (u32 i=0;i<base.w/size[0];i++) {
            Shird_Sprite tile = shird_sprite_crop(&base, (vec2u){i*size[0], j*size[1]}, (vec2u){size[0], size[1]});
            if (array_list_append(sprites, &tile) == -1)
                ERR_EXT("failed to add sprite to spritesheet: %s\n", path);
        }
    }

    return sprites;
}

void shird_spritesheet_free(Array_List *spr) {
    for (u32 i=0;i<spr->len;i++) {
        Shird_Sprite *val = array_list_get(spr, i);
        shird_sprite_free(val);
    }
    free(spr->items);
}

Shird_Animation shird_animation_create(char* path, vec2u size, f32 delay) {
	Array_List *spritesheet = shird_spritesheet_load(path, size);
	Shird_Animation out = (Shird_Animation) {
		.sprites = spritesheet,
		.index = 0,
		.delay = delay,
		.delay_counter = delay,
		.ended = false,
		.repeat = true,
	};
	return out;
}

void shird_animation_free(Shird_Animation *ani) {
	shird_spritesheet_free(ani->sprites);
}

////////////////////////
//       FONT
////////////////////////

Shird_Font shird_font_create(char *path) {
    Shird_Font out;
    out.sprites = array_list_create(sizeof(Shird_Sprite), 0);

    Shird_Sprite base = shird_sprite_load(path);
	u32 *pixels = base.data;
	u32 width = base.w;

	u32 height = base.h;
	out.char_size[0] = 0;
    out.char_size[1] = height;

	i32 start_x = -1;
	for (i32 x=0;x<width; x++) {
		bool is_red_line = pixels[x] == RED; 
		if (is_red_line) {
			if (start_x != -1) {
				i32 char_w = x - start_x;
				out.char_size[0] = char_w;
				u32 *char_pixels = malloc(sizeof(u32) * char_w * height);
				if (char_pixels == NULL) {
					ERR_EXT("failed to malloc mem for font");
				}
				for (int py = 0; py < height; py++) {
					memcpy(
						char_pixels + py * char_w,          // Destination
						pixels + start_x + py * width,      // Source
						sizeof(u32) * char_w              // Number of bytes to copy
					);
				}
				array_list_append(out.sprites, &(Shird_Sprite){.data = char_pixels, .w = char_w, .h= height});
				start_x = -1;
			}
		} else if (start_x == -1) {
			start_x = x;
		}
	}

    return out;
}

////////////////////////
//        TIME
////////////////////////

f32 shird_time_delta = 0;
f32 shird_time_now = 0;
f32 shird_time_last = 0;
f32 shird_time_frame_last = 0;
f32 shird_time_frame_time = 0;
f32 shird_time_frame_delay = 0;
u32 shird_time_frame_count = 0;
u32 shird_time_frame_rate = 0;

void shird_time_init(u32 frame_rate) {
	u32 shird_time_frame_rate = frame_rate;
	f32 shird_time_frame_delay = 1000.f/frame_rate;
}

void shird_time_update(void) {
	shird_time_now = (f32)SDL_GetTicks();
	shird_time_delta = (shird_time_now-shird_time_last)/1000.f;
	shird_time_last = shird_time_now;
	++shird_time_frame_count;

	if (shird_time_now - shird_time_frame_last >= 1000.f) {
		shird_time_frame_rate = shird_time_frame_count;
		shird_time_frame_count = 0;
		shird_time_frame_last = shird_time_now;
	}
}

void shird_time_update_late(void) {
	shird_time_frame_time = (f32)SDL_GetTicks() - shird_time_now;

	if (shird_time_frame_delay > shird_time_frame_time) {
		SDL_Delay(shird_time_frame_delay - shird_time_frame_time);
	}
}


////////////////////////
//       INPUT
////////////////////////

Shird_Key_Status shird_input_get(const char *name) {
	SDL_Keycode keycode = SDL_GetKeyFromName(name);
	if (keycode == SDLK_UNKNOWN)
		return SHIRD_KEY_UNKNOWN;

	SDL_Scancode scancode = SDL_GetScancodeFromKey(keycode);

	const u8* state = SDL_GetKeyboardState(NULL);

	if (state[scancode])
		return SHIRD_KEY_DOWN;
	else 
		return SHIRD_KEY_UP;
}

Shird_Mouse_Status shird_input_mouse_get(const char *button_name) {
    u32 button = 0;
    if (strcmp(button_name, "left") == 0) {
        button = SDL_BUTTON_LEFT;
    } else if (strcmp(button_name, "right") == 0) {
        button = SDL_BUTTON_RIGHT;
    } else if (strcmp(button_name, "middle") == 0) {
        button = SDL_BUTTON_MIDDLE;
    } else if (strcmp(button_name, "x1") == 0) {
        button = SDL_BUTTON_X1;
    } else if (strcmp(button_name, "x2") == 0) {
        button = SDL_BUTTON_X2;
    } else {
        return SHIRD_MOUSE_UNKNOWN;
    }

    u32 state = SDL_GetMouseState(NULL, NULL);

    if (state & SDL_BUTTON(button)) {
        return SHIRD_MOUSE_DOWN;
    } else {
        return SHIRD_MOUSE_UP;
    }
}

void shird_input_mouse(vec2i *vec) {
	SDL_GetMouseState(&(*vec)[0], &(*vec)[1]);
}


////////////////////////
//      PHYSICS
////////////////////////

bool shird_physics_intersect(Shird_AABB a, Shird_AABB b) {
	if (a.pos[0] > b.pos[0]+b.size[0] || b.pos[0] > a.pos[0]+a.size[0])
		return false;
	if (a.pos[1] > b.pos[1]+b.size[1] || b.pos[1] > a.pos[1]+a.size[1])
		return false;
	return true;
}

Shird_Direction aabb_collision_direction(Shird_AABB a, Shird_AABB b) {
    float axMin = a.pos[0];
    float axMax = a.pos[0] + a.size[0];
    float ayMin = a.pos[1];
    float ayMax = a.pos[1] + a.size[1];

    float bxMin = b.pos[0];
    float bxMax = b.pos[0] + b.size[0];
    float byMin = b.pos[1];
    float byMax = b.pos[1] + b.size[1];

    if (axMax < bxMin || axMin > bxMax || ayMax < byMin || ayMin > byMax) {
        return SHIRD_DIRECTION_NULL;
    }

    float overlapLeft = bxMax - axMin;
    float overlapRight = axMax - bxMin;
    float overlapTop = byMax - ayMin;
    float overlapBottom = ayMax - byMin;

    float minOverlap = overlapLeft;

    Shird_Direction direction = SHIRD_DIRECTION_LEFT;

    if (overlapRight < minOverlap) {
        minOverlap = overlapRight;
        direction = SHIRD_DIRECTION_RIGHT;
    }
    if (overlapTop < minOverlap) {
        minOverlap = overlapTop;
        direction = SHIRD_DIRECTION_UP;
    }
    if (overlapBottom < minOverlap) {
        minOverlap = overlapBottom;
        direction = SHIRD_DIRECTION_DOWN;
    }

    return direction;
}

////////////////////////
//        IO
////////////////////////


Shird_File shird_io_file_read(const char *path) {
	Shird_File file = {.is_valid=0};

	FILE *fp = fopen(path, "rb");
	if (!fp || ferror(fp))
		ERR_RET(file, SHIRD_IO_READ_ERROR_GENERAL, path, errno);

	char *data = NULL;
	char *tmp;
	size_t used = 0;
	size_t size = 0;
	size_t n;

	while (1) {
		if (used + SHIRD_IO_READ_CHUNK_SIZE+1 > size) {
			size = used+SHIRD_IO_READ_CHUNK_SIZE+1;

			if (size <= used) {
				free(data);
				ERR_RET(file, "input file too large: %s\n", path);
			}

			tmp = realloc(data, size);
			if (!tmp) {
				free(data);
				ERR_RET(file, SHIRD_IO_READ_ERROR_MEMORY, path);
			}
			data = tmp;
		}

		n = fread(data+used, 1, SHIRD_IO_READ_CHUNK_SIZE, fp);
		if (n == 0)
			break;

		used+=n;
	}
	
	if (ferror(fp)) {
		free(data);
		ERR_RET(file, SHIRD_IO_READ_ERROR_GENERAL, path, errno);
	}
	
	tmp = realloc(data, used+1);
	if (!tmp) {
		free(data);
		ERR_RET(file, SHIRD_IO_READ_ERROR_MEMORY, path);
	}
	data = tmp;
	data[used] = 0;

	file.data = data;
	file.len = used;
	file.is_valid = 1;

	return file;
}

u8 shird_io_file_write(void *buffer, size_t size, const char *path) {
	FILE *fp = fopen(path, "wb");
	if (!fp || ferror(fp)) 
		ERR_RET(1, "cannot write file : %s\n", path);
	size_t chunks_written = fwrite(buffer, size, 1, fp);
	fclose(fp);
	if (chunks_written != 1)
		ERR_RET(1, "write error: expected 1 chunk got %zu\n", chunks_written);

	return 0;
}
