#include "menu.h"
#include "c2d/text.h"
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct Menu_s {
  struct dirent **entries;
  int entries_len;
  int selected;
  C2D_TextBuf text_buf;
  C2D_Text *text_lines;
} *Menu;

int select_files(const struct dirent *entry) {
  // only keep regular files
  return entry->d_type == DT_REG;
}

Menu menu_create() {
  Menu menu = malloc(sizeof(struct Menu_s));
  menu->entries_len =
      scandir("sdmc:/3ds/chip8/roms/", &menu->entries, select_files, alphasort);
  menu->selected = 0;

  if (menu->entries_len > 0) {
    size_t total_len = 0;
    for (int n = 0; n < menu->entries_len; n++) {
      total_len += strlen(menu->entries[n]->d_name);
    }
    menu->text_buf = C2D_TextBufNew(total_len + 1);

    menu->text_lines = calloc(menu->entries_len, sizeof(C2D_Text));
    for (int n = 0; n < menu->entries_len; n++) {
      C2D_TextParse(&menu->text_lines[n], menu->text_buf,
                    menu->entries[n]->d_name);
      C2D_TextOptimize(&menu->text_lines[n]);
    }
  } else {
    const char *message;
    if (menu->entries_len == 0) {
      message = "/3ds/chip8/roms/ is empty";
    } else {
      message = "/3ds/chip8/roms/ could not be opened";
    }
    menu->text_buf = C2D_TextBufNew(strlen(message) + 1);
    menu->text_lines = malloc(sizeof(C2D_Text));
    C2D_TextParse(&menu->text_lines[0], menu->text_buf, message);
    C2D_TextOptimize(&menu->text_lines[0]);
    menu->entries = NULL;
    menu->entries_len = 1;
  }

  return menu;
}

void menu_destroy(Menu menu) {
  for (int n = 0; n < menu->entries_len; n++) {
    free(menu->entries[n]);
  }
  free(menu->entries);
  C2D_TextBufDelete(menu->text_buf);
  free(menu->text_lines);
  free(menu);
}

void menu_tick(Menu menu) {
  u32 kDown = hidKeysDown();
  if (kDown & KEY_DDOWN && menu->selected < menu->entries_len - 1) {
    menu->selected += 1;
  } else if (kDown & KEY_DUP && menu->selected > 0) {
    menu->selected -= 1;
  }
}

void menu_draw(Menu menu, C3D_RenderTarget *top, C3D_RenderTarget *bottom) {
  if (C3D_FrameBegin(C3D_FRAME_NONBLOCK)) {
    C2D_TargetClear(top, C2D_Color32(0, 0, 0, 255));
    C2D_SceneBegin(top);

    float line_height = 17.0;
    float top_screen_height = 240.0;
    float selected_pos = (top_screen_height - line_height) / 2.0;

    for (int n = 0; n < menu->entries_len; n++) {
      C2D_DrawText(&menu->text_lines[n], C2D_WithColor | C2D_AtBaseline, 40.0,
                   (n - menu->selected) * line_height + selected_pos, 0.0, 0.75,
                   0.75, C2D_Color32(255, 255, 255, 255));
    }

    C2D_DrawCircleSolid(30.0, selected_pos - line_height / 2.0, 0.0, 5.0,
                        C2D_Color32(255, 255, 255, 255));

    C2D_TargetClear(bottom, C2D_Color32(0, 0, 0, 255));
    C2D_SceneBegin(bottom);

    for (int n = 0; n < menu->entries_len; n++) {
      C2D_DrawText(&menu->text_lines[n], C2D_WithColor | C2D_AtBaseline, 0.0,
                   (n - menu->selected) * line_height + selected_pos -
                       top_screen_height,
                   0.0, 0.75, 0.75, C2D_Color32(255, 255, 255, 255));
    }

    C2D_Flush();
    C3D_FrameEnd(0);
  }
}

void menu_read_rom(Menu menu, Rom *rom) {
  if (menu->entries == NULL) {
    // directory couldn't be opened or is empty
    return;
  }

  struct dirent *entry = menu->entries[menu->selected];

  if (chdir("sdmc:/3ds/chip8/roms/") == -1) {
    return;
  }

  FILE *file = fopen(entry->d_name, "rb");
  if (file == NULL) {
    return;
  }

  if (fseek(file, 0, SEEK_END) == -1) {
    return;
  }

  int length = ftell(file);
  if (length == -1) {
    return;
  }

  if (fseek(file, 0, SEEK_SET) == -1) {
    return;
  }

  u8 *buffer = malloc(length);
  if (fread(buffer, 1, length, file) == length) {
    // successfully read entire file
    rom->len = length;
    rom->ptr = buffer;
  }

  fclose(file);
}

Rom menu_run(C3D_RenderTarget *top, C3D_RenderTarget *bottom) {
  Rom rom = {.ptr = NULL};
  Menu menu = menu_create();

  while (aptMainLoop()) {
    hidScanInput();

    u32 kDown = hidKeysDown();
    if (kDown & KEY_B) {
      // quit
      break;
    }
    if (kDown & KEY_A) {
      // select current file
      menu_read_rom(menu, &rom);
      break;
    }

    menu_tick(menu);
    menu_draw(menu, top, bottom);
  }

  menu_destroy(menu);
  return rom;
}
