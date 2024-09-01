#include "system.h"
#include "display.h"
#include "font.h"
#include <stdlib.h>
#include <string.h>

struct System_s {
  Display display;
  u8 *memory;
  u16 counter;
  u16 index;
  u8 registers[16];
  u16 stack[16];
  u8 stack_len;
};

const size_t memory_size = 0x1000;
const size_t font_offset = 0x0050;

System system_create() {
  System system = malloc(sizeof(struct System_s));
  system->display = display_create();
  system->memory = calloc(memory_size, sizeof(u8));
  system->counter = 0x200;
  system->index = 0;
  memset(system->registers, 0, sizeof(u8[16]));
  memset(system->stack, 0, sizeof(u16[16]));
  system->stack_len = 0;

  memcpy(system->memory + font_offset, font, font_len);

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
  if (system->counter >= memory_size) {
    // reached end of memory, halt
    return;
  }

  u16 instruction = (system->memory[system->counter] << 8) |
                    system->memory[system->counter + 1];
  system->counter += 2;

  switch (instruction >> 12) {
  case 0x0: {
    switch (instruction) {
    case 0x00E0: {
      // 00E0: clear screen
      display_clear(system->display);
      break;
    }
    case 0x00EE: {
      // 00EE: return
      if (system->stack_len > 0) {
        system->stack_len--;
        system->counter = system->stack[system->stack_len];
      }
      break;
    }
    }
    break;
  }
  case 0x1: {
    // 1NNN: jump
    system->counter = instruction & 0x0FFF;
    break;
  }
  case 0x2: {
    // 2NNN: call subroutine
    if (system->stack_len < 15) {
      system->stack[system->stack_len] = system->counter;
      system->stack_len++;
      system->counter = instruction & 0x0FFF;
    }
    break;
  }
  case 0x3: {
    // 3XNN: skip if equal
    u8 reg = (instruction & 0x0F00) >> 8;
    u8 value = instruction & 0x00FF;
    if (system->registers[reg] == value) {
      system->counter += 2;
    }
    break;
  }
  case 0x4: {
    // 4XNN: skip if not equal
    u8 reg = (instruction & 0x0F00) >> 8;
    u8 value = instruction & 0x00FF;
    if (system->registers[reg] != value) {
      system->counter += 2;
    }
    break;
  }
  case 0x5: {
    // 5XY0: skip if reg equal
    u8 x_reg = (instruction & 0x0F00) >> 8;
    u8 y_reg = (instruction & 0x00F0) >> 4;
    if (system->registers[x_reg] == system->registers[y_reg]) {
      system->counter += 2;
    }
    break;
  }
  case 0x6: {
    // 6XNN: set register
    u8 reg = (instruction & 0x0F00) >> 8;
    system->registers[reg] = instruction & 0x00FF;
    break;
  }
  case 0x7: {
    // 7XNN: add const to register
    u8 reg = (instruction & 0x0F00) >> 8;
    system->registers[reg] += instruction & 0x00FF;
    break;
  }
  case 0x8: {
    switch (instruction & 0x000F) {
    case 0x0: {
      // 8XY0: copy register
      u8 x_reg = (instruction & 0x0F00) >> 8;
      u8 y_reg = (instruction & 0x00F0) >> 4;
      system->registers[x_reg] = system->registers[y_reg];
      break;
    }
    case 0x1: {
      // 8XY1: or registers
      u8 x_reg = (instruction & 0x0F00) >> 8;
      u8 y_reg = (instruction & 0x00F0) >> 4;
      system->registers[x_reg] |= system->registers[y_reg];
      break;
    }
    case 0x2: {
      // 8XY2: and registers
      u8 x_reg = (instruction & 0x0F00) >> 8;
      u8 y_reg = (instruction & 0x00F0) >> 4;
      system->registers[x_reg] &= system->registers[y_reg];
      break;
    }
    case 0x3: {
      // 8XY3: xor registers
      u8 x_reg = (instruction & 0x0F00) >> 8;
      u8 y_reg = (instruction & 0x00F0) >> 4;
      system->registers[x_reg] ^= system->registers[y_reg];
      break;
    }
    case 0x4: {
      // 8XY4: add registers
      u8 x_reg = (instruction & 0x0F00) >> 8;
      u8 y_reg = (instruction & 0x00F0) >> 4;
      u8 x_old = system->registers[x_reg];
      system->registers[x_reg] += system->registers[y_reg];
      if (system->registers[x_reg] < x_old) {
        system->registers[0xF] = 1;
      } else {
        system->registers[0xF] = 0;
      }
      break;
    }
    case 0x5: {
      // 8XY5: subtract registers
      u8 x_reg = (instruction & 0x0F00) >> 8;
      u8 y_reg = (instruction & 0x00F0) >> 4;
      u8 x_old = system->registers[x_reg];
      system->registers[x_reg] -= system->registers[y_reg];
      if (system->registers[y_reg] > x_old) {
        system->registers[0xF] = 0;
      } else {
        system->registers[0xF] = 1;
      }
      break;
    }
    case 0x6: {
      // 8XY6: right shift
      u8 x_reg = (instruction & 0x0F00) >> 8;
      u8 y_reg = (instruction & 0x00F0) >> 4;
      u8 old_y = system->registers[y_reg];
      system->registers[x_reg] = old_y >> 1;
      system->registers[0xF] = old_y & 1;
      break;
    }
    case 0x7: {
      // 8XY7: reverse subtract
      u8 x_reg = (instruction & 0x0F00) >> 8;
      u8 y_reg = (instruction & 0x00F0) >> 4;
      u8 x_old = system->registers[x_reg];
      system->registers[x_reg] = system->registers[y_reg] - x_old;
      if (system->registers[y_reg] < x_old) {
        system->registers[0xF] = 0;
      } else {
        system->registers[0xF] = 1;
      }
      break;
    }
    case 0xE: {
      // 8XYE: left shift
      u8 x_reg = (instruction & 0x0F00) >> 8;
      u8 y_reg = (instruction & 0x00F0) >> 4;
      u8 old_y = system->registers[y_reg];
      system->registers[x_reg] = old_y << 1;
      system->registers[0xF] = old_y >> 7;
      break;
    }
    }
    break;
  }
  case 0x9: {
    // 9XY0: skip if reg not equal
    u8 x_reg = (instruction & 0x0F00) >> 8;
    u8 y_reg = (instruction & 0x00F0) >> 4;
    if (system->registers[x_reg] != system->registers[y_reg]) {
      system->counter += 2;
    }
    break;
  }
  case 0xA: {
    // ANNN: set index
    system->index = instruction & 0x0FFF;
    break;
  }
  case 0xB: {
    // BNNN: jump with offset
    system->counter = (instruction & 0x0FFF) + system->registers[0];
    break;
  }
  case 0xC: {
    // CXNN: random with mask
    u8 reg = (instruction & 0x0F00) >> 8;
    system->registers[reg] = random() & (instruction & 0x00FF);
    break;
  }
  case 0xD: {
    // DXYN: draw sprite
    u8 x_reg = (instruction & 0x0F00) >> 8;
    u8 y_reg = (instruction & 0x00F0) >> 4;
    u8 height = instruction & 0x000F;

    u8 x = system->registers[x_reg] % 64;
    u8 y = system->registers[y_reg] % 32;

    system->registers[0xF] = 0;

    for (int row = 0; row < height; row++) {
      if (y + row >= 32) {
        break;
      }
      u8 data = system->memory[system->index + row];
      for (int col = 0; col < 8; col++) {
        if (x + col >= 64) {
          break;
        }
        if (((data >> (7 - col)) & 1) == 1) {
          bool hit = display_toggle_pixel(system->display, x + col, y + row);
          if (hit)
            system->registers[0xF] = 1;
        }
      }
    }
    break;
  }
  case 0xE: {
    switch (instruction & 0x00FF) {
    case 0x9E: {
      // EX9E: skip if pressed
      // TODO
      break;
    }
    case 0xA1: {
      // EXA1: skip if not pressed
      // TODO
      break;
    }
    }
    break;
  }
  case 0xF: {
    switch (instruction & 0x00FF) {
    case 0x07: {
      // FX07: read delay
      // TODO
      break;
    }
    case 0x0A: {
      // FX0A: wait for key
      // TODO
      break;
    }
    case 0x15: {
      // FX15: set delay
      // TODO
      break;
    }
    case 0x18: {
      // FX18: set buzzer
      // TODO
      break;
    }
    case 0x1E: {
      // FX1E: add to index
      u8 x_reg = (instruction & 0x0F00) >> 8;
      system->index += system->registers[x_reg];
      break;
    }
    case 0x29: {
      // FX29: set index to character
      u8 x_reg = (instruction & 0x0F00) >> 8;
      system->index =
          font_offset + (system->registers[x_reg] & 0x0F) * font_stride;
      break;
    }
    case 0x33: {
      // FX33: write binary-coded decimal
      u8 x_reg = (instruction & 0x0F00) >> 8;
      u8 value = system->registers[x_reg];

      system->memory[system->index + 2] = value % 10;
      value /= 10;
      system->memory[system->index + 1] = value % 10;
      value /= 10;
      system->memory[system->index + 0] = value % 10;

      break;
    }
    case 0x55: {
      // FX55: save registers
      u8 x = (instruction & 0x0F00) >> 8;
      for (int reg = 0; reg <= x; reg++) {
        system->memory[system->index + reg] = system->registers[reg];
      }
      break;
    }
    case 0x65: {
      // FX65: load registers
      u8 x = (instruction & 0x0F00) >> 8;
      for (int reg = 0; reg <= x; reg++) {
        system->registers[reg] = system->memory[system->index + reg];
      }
      break;
    }
    }
    break;
  }
  }
}

void system_draw(System system) { display_draw(system->display); }
