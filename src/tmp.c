#include <raylib.h>
#include <raymath.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "util.h"
#include "structures.h"

//costants
#define SCREEN_W (320*3)
#define SCREEN_H (180*3)

typedef struct {
  Vector2 pos;
  f32 radius;
  Color color;
} Light;

RenderTexture lightmap;

void draw_shadow(Vector2 lightp, Vector2 line_start, Vector2 line_end) {
  Vector2 p1, p2, p3, p4;
  p1 = line_start;
  p2 = line_end;
  p3 = (Vector2){line_start.x+15*(line_start.x-lightp.x), line_start.y+15*(line_start.y-lightp.y)};
  p4 = (Vector2){line_end.x+15*(line_end.x-lightp.x), line_end.y+15*(line_end.y-lightp.y)};

  /*
  DrawLineV(p1, p2, WHITE);
  DrawLineV(p2, p4, WHITE);
  DrawLineV(p4, p3, WHITE);
  DrawLineV(p3, p1, WHITE);
  */

  DrawTriangle(p1, p2, p4, (Color){33, 46, 70, 255});
  DrawTriangle(p4, p3, p1, (Color){33, 46, 70, 255});
}

int main() {
  InitWindow(SCREEN_W, SCREEN_H, "fuck");
  SetTargetFPS(60);

  Rectangle box = (Rectangle){100, 100, 100, 100};

  Image image = GenImageGradientLinear(100, 50, 0, WHITE, BLACK);
  Texture2D text = LoadTextureFromImage(image);
  UnloadImage(image);

  Light l = (Light) {
    {SCREEN_W/2, SCREEN_H/2},
    350,
    (Color){255, 255, 255, 150}
  };
  lightmap = LoadRenderTexture(SCREEN_W, SCREEN_H);

  while (!WindowShouldClose()) {
    l.pos = GetMousePosition();
    

    BeginTextureMode(lightmap);
    ClearBackground((Color){33, 46, 70, 255});

    DrawCircleGradient(l.pos.x, l.pos.y, l.radius, l.color, (Color){33, 46, 70, 255});
    //DrawCircle(l.pos.x, l.pos.y, l.radius, l.color);

    draw_shadow(l.pos, (Vector2){box.x, box.y}, (Vector2){box.x+100, box.y});
    draw_shadow(l.pos, (Vector2){box.x+100, box.y}, (Vector2){box.x+100, box.y+100});
    draw_shadow(l.pos, (Vector2){box.x+100, box.y+100}, (Vector2){box.x, box.y+100});
    draw_shadow(l.pos, (Vector2){box.x, box.y+100}, (Vector2){box.x, box.y});

    EndTextureMode();


    BeginDrawing();
    ClearBackground(WHITE);
    

    DrawRectangleRec(box, RED);
    DrawTexture(text, 0, 0, WHITE);


    BeginBlendMode(BLEND_MULTIPLIED);
    DrawTextureRec(lightmap.texture, (Rectangle){0, 0, lightmap.texture.width, -lightmap.texture.height}, (Vector2){0, 0}, WHITE);
    EndBlendMode();

    //debug shit
    DrawLine(l.pos.x, l.pos.y, box.x, box.y, GREEN);
    DrawLine(l.pos.x, l.pos.y, box.x+100, box.y, GREEN);
    DrawLine(l.pos.x, l.pos.y, box.x+100, box.y+100, GREEN);
    DrawLine(l.pos.x, l.pos.y, box.x, box.y+100, GREEN);


    EndDrawing();
  }

  CloseWindow();

  return 0;
}
