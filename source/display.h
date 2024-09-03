#pragma once

#include <3ds.h>

typedef struct Display_s *Display;

Display display_create();
void display_destroy(Display);

void display_draw(Display);
bool display_toggle_pixel(Display display, u8 x, u8 y);
void display_clear(Display);
