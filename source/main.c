#include "c2d/base.h"
#include "display.h"

int main(int argc, char *argv[]) {
  gfxInitDefault();
  C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
  C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
  C2D_Prepare();

  C3D_RenderTarget *top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
  Display display = display_create();

  while (aptMainLoop()) {
    hidScanInput();

    u32 kDown = hidKeysDown();
    if (kDown & KEY_START)
      break;

    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    C2D_TargetClear(top, C2D_Color32(0, 0, 0, 255));
    C2D_SceneBegin(top);

    display_draw(display);

    C2D_Flush();
    C3D_FrameEnd(0);
  }

  display_destroy(display);
  C2D_Fini();
  C3D_Fini();
  gfxExit();
  return 0;
}
