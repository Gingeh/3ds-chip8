#include "3ds/gfx.h"
#include "c2d/base.h"
#include "keypad.h"
#include "rom.h"
#include "system.h"
#include <stdlib.h>

int main(int argc, char *argv[]) {
  gfxInitDefault();
  C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
  C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
  C2D_Prepare();
  srand(osGetTime());

  C3D_RenderTarget *top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
  C3D_RenderTarget *bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

  System system = system_create();
  system_load_rom(system, rom, rom_len);
  keypad_init();

  while (aptMainLoop()) {
    hidScanInput();

    u32 kDown = hidKeysDown();
    if (kDown & KEY_START)
      break;

    system_tick(system);

    if (C3D_FrameBegin(C3D_FRAME_NONBLOCK)) {
      C2D_TargetClear(top, C2D_Color32(0, 0, 0, 255));
      C2D_SceneBegin(top);

      system_draw(system);

      C2D_TargetClear(bottom, C2D_Color32(0, 0, 0, 255));
      C2D_SceneBegin(bottom);

      keypad_draw();

      C2D_Flush();
      C3D_FrameEnd(0);
    }
  }

  system_destroy(system);
  keypad_fini();
  C2D_Fini();
  C3D_Fini();
  gfxExit();
  return 0;
}
