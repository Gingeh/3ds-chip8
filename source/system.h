#pragma once

#include "3ds/types.h"
#include "c3d/renderqueue.h"

typedef struct {
  u8 *ptr;
  size_t len;
} Rom;

void system_run(Rom rom, C3D_RenderTarget *top, C3D_RenderTarget *bottom);
