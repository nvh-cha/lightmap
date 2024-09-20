#include <raylib.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "util.h"
#include "structures.h"

//costants
#define SCREEN_W (320*3)
#define SCREEN_H (180*3)
#define MAX_LIGHTS 1
#define MAX_BOXES 2

typedef struct {
  Vector2 pos;
  f32 radius;
  Color color;
} Light;

Light lights[MAX_LIGHTS];
Rectangle boxes[MAX_BOXES];

void create_lights(void) {
  lights[0] = (Light){{SCREEN_W/2, SCREEN_H/2}, 100, GREEN};
}

void create_boxes(void) {
  boxes[0] = (Rectangle){200, 200, 100, 100};
}

RenderTexture lightmap;

int main() {
  InitWindow(SCREEN_W, SCREEN_H, "fuck");
  SetTargetFPS(60);

  create_lights();
  create_boxes();

  lightmap = LoadRenderTexture(SCREEN_W, SCREEN_H);

  while (!WindowShouldClose()) {
    lights[0].pos = GetMousePosition();

    BeginTextureMode(lightmap);
    ClearBackground((Color){0, 0, 0, 0});
    
    for (u8 i=0;i<MAX_LIGHTS;i++) {
      DrawCircleGradient(lights[i].pos.x, lights[i].pos.y, lights[i].radius, lights[i].color, (Color){lights[i].color.r, lights[i].color.g, lights[i].color.b, 0});
    }

    EndTextureMode();

    BeginDrawing();
    ClearBackground(BLACK);
    /*
    
    for (u8 i=0;i<MAX_BOXES;i++) {
      DrawRectangleRec(boxes[i], RED);
    }


    BeginBlendMode(BLEND_ADDITIVE);
    DrawTextureRec(lightmap.texture, (Rectangle){0, 0, lightmap.texture.width, -lightmap.texture.height}, (Vector2){0, 0}, WHITE);
    EndBlendMode();
    */
    DrawTextureRec(lightmap.texture, (Rectangle){0, 0, lightmap.texture.width, -lightmap.texture.height}, (Vector2){0, 0}, WHITE);
    EndDrawing();
  }

  CloseWindow();

  return 0;
}
