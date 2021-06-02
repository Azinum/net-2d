// renderer.h

#ifndef _RENDERER_H
#define _RENDERER_H

#include "image.h"

typedef struct Render_state {
  Image framebuffer;
} Render_state;

extern Render_state render_state;

i32 renderer_init(struct Render_state* renderer, i32 width, i32 height);

void renderer_swap_buffers(struct Render_state* renderer);

void renderer_free(struct Render_state* renderer);

#endif
