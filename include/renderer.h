// renderer.h

#ifndef _RENDERER_H
#define _RENDERER_H

#include "image.h"
#include "math_util.h"

typedef struct Render_state {
  Image fb[2];
  Image* framebuffer;
  u8 fb_target;
} Render_state;

extern Render_state render_state;

i32 renderer_init(struct Render_state* renderer, i32 width, i32 height);

void renderer_swap_buffers(struct Render_state* renderer);

void render_quad(const v2 p, const v2 size, Color_rgba color);

void render_quad_border(const v2 p, const v2 size, Color_rgba color);

void render_image(Image* image, const v2 p, const v2 size, const v2 uv_offset, const v2 uv_range, Color_rgba tint);

void render_sprite(u32 spritesheet_id, u32 sprite_id, const v2 p, const v2 size, Color_rgba tint);

void renderer_free(struct Render_state* renderer);

#endif
