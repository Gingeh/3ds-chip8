#include <3ds.h>

typedef struct System_s *System;

System system_create();
void system_destroy(System);

void system_load_rom(System, u8 *, size_t);
void system_tick(System);
void system_draw(System);
