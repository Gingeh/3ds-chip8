#include "c2d/base.h"
#include "keypad.h"
#include "menu.h"
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

  keypad_init();

  while (aptMainLoop()) {
    // run menu unil complete
    Rom rom = menu_run(top, bottom);

    // menu returns no rom if requested to exit (by apt or user)
    if (rom.ptr == NULL)
      break;

    // run system with rom until requested to exit
    system_run(rom, top, bottom);
    free(rom.ptr);
    // if the player choses to exit then the loop will restart
    // if the apt tells the system to exit then the loop will end
  }

  keypad_fini();
  C2D_Fini();
  C3D_Fini();
  gfxExit();
  return 0;
}
