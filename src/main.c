#include "engine.h"

#define SCREEN_W (320*3)
#define SCREEN_H (180*3)

RenderTexture lightmap;

int main() {
  InitWindow(SCREEN_W, SCREEN_H, "fuck python");
  SetTargetFPS(60);

  lightmap = LoadRenderTexture(SCREEN_W, SCREEN_H);
  Light light = (Light) {
    .pos = {0, 0},
    .color = WHITE,
    .radius = 250
  };

  while (!WindowShouldClose()) {
    BeginTextureMode(lightmap);
    BeginBlendMode(BLEND_ALPHA);
    ClearBackground((Color){20, 20, 20, 200});
    
    light.pos = GetMousePosition();
    light_render(&light);
    
    EndBlendMode();
    EndTextureMode();

    DrawRectangle(SCREEN_W/2-50, SCREEN_H/2-50, 100, 100, YELLOW);

    BeginDrawing();
    ClearBackground(BLACK);

    lightmap_render(&lightmap);
    EndDrawing();
  }
  
  CloseWindow();
  return 0;
}
