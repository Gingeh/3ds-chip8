#include "keypad.h"
#include "c2d/spritesheet.h"
#include "tex_t3x.h"

C2D_SpriteSheet spritesheet;
C2D_Image keypad_img;
int recent_key;

const int table[4][4] = {
    {0x1, 0x2, 0x3, 0xC},
    {0x4, 0x5, 0x6, 0xD},
    {0x7, 0x8, 0x9, 0xE},
    {0xA, 0x0, 0xB, 0xF},
};

void keypad_init() {
  spritesheet = C2D_SpriteSheetLoadFromMem(tex_t3x, tex_t3x_size);
  keypad_img = C2D_SpriteSheetGetImage(spritesheet, 0);
  C3D_TexSetFilter(keypad_img.tex, GPU_NEAREST, GPU_NEAREST);
  recent_key = -1;
}

void keypad_fini() { C2D_SpriteSheetFree(spritesheet); }

void keypad_draw() {
  C2D_DrawImageAt(keypad_img, 64.0, 12.0, 0.0, NULL, 6.0, 6.0);
}

int keypad_held() {
  if (!(hidKeysHeld() & KEY_TOUCH))
    return -1;

  touchPosition pos = {0};
  hidTouchRead(&pos);

  // check if touch inside bounding box
  if (pos.px < 64 || pos.px >= 256 || pos.py < 12 || pos.py >= 228)
    return -1;

  u16 x = (pos.px - 64) / (6 * 8);
  u16 y = (pos.py - 12) / (6 * 9);

  return table[y][x];
}

int keypad_released() {
  if (hidKeysUp() & KEY_TOUCH) {
    return recent_key;
  } else {
    recent_key = keypad_held();
    return -1;
  }
}
