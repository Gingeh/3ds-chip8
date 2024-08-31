#include "display.h"
#include "c2d/base.h"
#include <stdlib.h>

struct Display_s {
  C2D_Image image;
  C2D_Image dummy;
  u16 *pixel_buffer;
  bool dirty;
};

Display display_create() {
  // 64x32, 16-bit colour, no interpolation
  C3D_Tex *display_tex = malloc(sizeof(C3D_Tex));
  C3D_TexInit(display_tex, 64, 32, GPU_RGB565);
  C3D_TexSetFilter(display_tex, GPU_NEAREST, GPU_NEAREST);

  // using the whole texture, 64x32 region with 0-1 UVs
  Tex3DS_SubTexture *display_subtex = malloc(sizeof(Tex3DS_SubTexture));
  display_subtex->width = 64;
  display_subtex->height = 32;
  display_subtex->top = 1.0;
  display_subtex->bottom = 0.0;
  display_subtex->left = 0.0;
  display_subtex->right = 1.0;

  // image to be drawn
  C2D_Image display_image = {.tex = display_tex, .subtex = display_subtex};

  // empty dummy image, workaround to forcefully unbind the display texture
  C3D_Tex *dummy_tex = malloc(sizeof(C3D_Tex));
  C3D_TexInit(dummy_tex, 0, 0, GPU_L4);
  Tex3DS_SubTexture *dummy_subtex = malloc(sizeof(Tex3DS_SubTexture));
  C2D_Image dummy = {.tex = dummy_tex, .subtex = dummy_subtex};

  Display display = malloc(sizeof(struct Display_s));
  display->image = display_image;
  display->dummy = dummy;
  display->pixel_buffer = linearAlloc(64 * 32 * sizeof(u16));
  memset(display->pixel_buffer, 0, 64 * 32 * sizeof(u16));
  display->dirty = true;

  return display;
}

void display_destroy(Display display) {
  C3D_TexDelete(display->image.tex);
  free(display->image.tex);
  free((void *)display->image.subtex);
  linearFree(display->pixel_buffer);
  free(display);
}

void display_draw(Display display) {
  if (display->dirty) {
    // workaround to forcefully unbind the display texture
    C2D_DrawImageAt(display->dummy, 0.0, 0.0, 0.0, NULL, 0.0, 0.0);

    C3D_SyncDisplayTransfer(
        (u32 *)display->pixel_buffer, GX_BUFFER_DIM(64, 32),
        (u32 *)display->image.tex->data, GX_BUFFER_DIM(64, 32),
        (GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(1) |
         GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGB565) |
         GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB565) |
         GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO)));

    display->dirty = false;
  }

  C2D_DrawImageAt(display->image, 8.0, 24.0, 0.0, NULL, 6., 6.);
}

bool display_toggle_pixel(Display display, u8 x, u8 y) {
  u16 index = x + y * 64;
  u16 *pixel = &display->pixel_buffer[index];
  *pixel = ~(*pixel);
  display->dirty = true;
  return *pixel == 0;
}

void display_clear(Display display) {
  memset(display->pixel_buffer, 0, 64 * 32 * sizeof(u16));
  display->dirty = true;
}
