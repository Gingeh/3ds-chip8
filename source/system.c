#include "system.h"
#include "display.h"
#include <stdlib.h>
#include <string.h>

struct System_s {
  Display display;
  u8 *memory;
  u16 counter;
  u16 index;
  u8 registers[16];
};

const size_t memory_size = 0x1000;

System system_create() {
  System system = malloc(sizeof(struct System_s));
  system->display = display_create();
  system->memory = calloc(memory_size, sizeof(u8));
  system->counter = 0x200;
  system->index = 0;
  memset(system->registers, 0, sizeof(u8[16]));

  return system;
}

void system_destroy(System system) {
  display_destroy(system->display);
  free(system->memory);
  free(system);
}

void system_load_rom(System system, u8 *rom, size_t size) {
  if (size > memory_size - 0x200)
    size = memory_size - 0x200;
  memcpy(system->memory + 0x200, rom, size);
}

void system_tick(System system) {
  u16 instruction = (system->memory[system->counter] << 8) |
                    system->memory[system->counter + 1];
  system->counter += 2;

  switch (instruction >> 12) {
  case 0x0:
    switch (instruction) {
    case 0x00E0:
      // 00E0: clear screen
      display_clear(system->display);
      break;
    case 0x00EE:
      // 00EE: return
      break;
    }
    break;
  case 0x1:
    // 1NNN: jump
    system->counter = instruction & 0x0FFF;
    break;
  case 0x6:; // <- https://stackoverflow.com/a/18496414/11679277
    // 6XNN: set register
    u8 reg = (instruction & 0x0F00) >> 8;
    system->registers[reg] = instruction & 0x00FF;
    break;
  case 0xA:
    // ANNN: set index
    system->index = instruction & 0x0FFF;
    break;
  case 0xD:;
    // DXYN: draw sprite
    u8 x_reg = (instruction & 0x0F00) >> 8;
    u8 y_reg = (instruction & 0x00F0) >> 4;
    u8 height = instruction & 0x000F;

    u8 x = system->registers[x_reg];
    u8 y = system->registers[y_reg];

    for (int row = 0; row < height; row++) {
      u8 data = system->memory[system->index + row];
      for (int col = 0; col < 8; col++) {
        if (((data >> col) & 1) == 1)
          display_toggle_pixel(system->display, x + 7 - col, y + row);
      }
    }
    break;
  }
}

void system_draw(System system) { display_draw(system->display); }
