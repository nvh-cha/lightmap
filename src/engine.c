#include "engine.h"

// util
f32 distance(vec2 a, vec2 b) {
  return sqrt(pow(b.x - a.x, 2)+pow(a.y-b.y, 2));
}

// light
void light_render(Light *l) {
  DrawCircleGradient(l->pos.x, l->pos.y, l->radius, l->color, (Color){0, 0, 0, 0});
}
void light_shadow(Light l, Line line, Color color) {
  // p0 = start
  // p1 = end
  // p2 = start+15*(start-light)
  // p3 = end+15*(end-light)
  // PS change the 15 if too short

  vec2 p0, p1, p2, p3;
  p0 = line.start;
  p1 = line.end;
  p2 = (vec2){line.start.x+15*(line.start.x-l.pos.x),
    line.start.y+15*(line.start.y-l.pos.y)};
  p3 = (vec2){line.end.x+15*(line.end.x-l.pos.x),
    line.end.y+15*(line.end.y-l.pos.y)};

  DrawTriangle(p0, p1, p3, color);
  DrawTriangle(p3, p2, p0, color);
}

void light_shadow_rect(Light l, Rectangle rect, Color color) {
  Line lines[4];
  lines[0] = (Line){{rect.x, rect.y}, {rect.x+rect.width, rect.y}};
  lines[1] = (Line){{rect.x+rect.width, rect.y}, {rect.x+rect.width, rect.y+rect.height}};
  lines[2] = (Line){{rect.x+rect.width, rect.y+rect.height}, {rect.x, rect.y+rect.height}};
  lines[3] = (Line){{rect.x, rect.y+rect.height}, {rect.x, rect.y}};
  
  bool tmp = false;
  for (u8 i=0;i<4;i++) {
    if (distance(lines[i].start, l.pos) < l.radius) {
      tmp = true;
    }
  }
  if (tmp) {
    light_shadow(l, lines[0], color);
    light_shadow(l, lines[1], color);
    light_shadow(l, lines[2], color);
    light_shadow(l, lines[3], color);
  }
}

void lightmap_render(RenderTexture *lightmap) {
  DrawTextureRec(lightmap->texture, (Rectangle){0, 0, lightmap->texture.width, -lightmap->texture.height}, (vec2){0, 0}, WHITE);
}

// file io
File io_file_read(const char *path) {
	File file = {.is_valid=0};

	FILE *fp = fopen(path, "rb");
	if (!fp || ferror(fp))
		ERR_RET(file, IO_READ_ERROR_GENERAL, path, errno);

	char *data = NULL;
	char *tmp;
	size_t used = 0;
	size_t size = 0;
	size_t n;

	while (1) {
		if (used + IO_READ_CHUNK_SIZE+1 > size) {
			size = used+IO_READ_CHUNK_SIZE+1;

			if (size <= used) {
				free(data);
				ERR_RET(file, "input file too large: %s\n", path);
			}

			tmp = realloc(data, size);
			if (!tmp) {
				free(data);
				ERR_RET(file, IO_READ_ERROR_MEMORY, path);
			}
			data = tmp;
		}

		n = fread(data+used, 1, IO_READ_CHUNK_SIZE, fp);
		if (n == 0)
			break;

		used+=n;
	}
	
	if (ferror(fp)) {
		free(data);
		ERR_RET(file, IO_READ_ERROR_GENERAL, path, errno);
	}
	
	tmp = realloc(data, used+1);
	if (!tmp) {
		free(data);
		ERR_RET(file, IO_READ_ERROR_MEMORY, path);
	}
	data = tmp;
	data[used] = 0;

	file.data = data;
	file.len = used;
	file.is_valid = 1;

	return file;
}

u8 io_file_write(void *buffer, size_t size, const char *path) {
	FILE *fp = fopen(path, "wb");
	if (!fp || ferror(fp)) 
		ERR_RET(1, "cannot write file : %s\n", path);
	size_t chunks_written = fwrite(buffer, size, 1, fp);
	fclose(fp);
	if (chunks_written != 1)
		ERR_RET(1, "write error: expected 1 chunk got %zu\n", chunks_written);

	return 0;
}

ArrayList arraylist_create(size_t item_size, size_t initial_capacity) {
  ArrayList arr = (ArrayList){
    .len = 0,
    .item_size = item_size,
    .capacity = initial_capacity
  };
  void *items = malloc(item_size*initial_capacity);
  if (!items) ERR_EXT("(arraylist) failed to allocate memory\n");
  arr.items = items;

  return arr;
}

// arraylist
bool arraylist_append(ArrayList *arr, void *item) {
  if (arr->capacity == arr->len) {
    //full
    if (arr->capacity == 0) 
      arr->capacity = 1;
    else
      arr->capacity *= 2;

    void *items = realloc(arr->items, arr->item_size*arr->capacity);
    if (!items) ERR_RET(true, "(arraylist) failed to reallocate memory\n");
    arr->items = items;
  }

  size_t index = arr->len++;
  memcpy(arr->items+arr->item_size*index, item, arr->item_size);

  return false;
}

void *arraylist_get(ArrayList *arr, size_t i) {
  if (i == arr->len) 
    ERR_EXT("(arraylist) index of bounds: %i\n", i);

  return arr->items+i*arr->item_size;
}

bool arraylist_remove(ArrayList *arr, size_t i) {
  if (arr->len == 0)
    ERR_RET(true, "(arraylist) the arr is empty\n");
  if (i >= arr->len)
    ERR_RET(true, "(arraylist) index out of bounds\n");
  
  --arr->len;

  u8 *item_ptr = (u8*)arr->items+i*arr->item_size;
  u8 *end_ptr = (u8*)arr->items+arr->len*arr->item_size;
  memcpy(item_ptr, end_ptr, arr->item_size);

  return false;
}

//spritesheet
Spritesheet spritesheet_create(vec2 size, char *path) {
  Spritesheet res = {0};
  
  // load the original image
  Image atlas = LoadImage(path);
  res.len = ((atlas.width/size.x) * (atlas.height/size.y)); // calclulate the len
 
  //allocate memory for the spritesheet
  res.sprites = malloc(res.len*sizeof(Texture2D));
  
  for (u32 y=0;y<(atlas.height/size.y);y++) {
    for (u32 x=0;x<(atlas.width/size.x);x++) {
      Image copy = ImageCopy(atlas);
      ImageCrop(&copy, (Rectangle){
        x*size.x, y*size.y, size.x, size.y
      });
      Texture2D tile = LoadTextureFromImage(copy);
      u32 slot = x+y*(atlas.width/size.x);
      memcpy(res.sprites+slot*sizeof(Texture2D), &tile, sizeof(Texture2D));
      UnloadImage(copy);
    }
  }

  UnloadImage(atlas);
  return res;
}

Texture2D spritesheet_get(Spritesheet s, u32 i) {
  if (i>=s.len) ERR_EXT("(spritesheet) index out of bounds: %i\n", i);

  return *(Texture2D*)(s.sprites+i*sizeof(Texture2D));
}

void spritesheet_free(Spritesheet *s) {
  for (u32 i=0;i<s->len;i++) {
    UnloadTexture(spritesheet_get(*s, i));
  }
  free(s->sprites);
  s->sprites = NULL;
}
