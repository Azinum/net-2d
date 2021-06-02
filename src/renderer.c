// renderer.c

#include "common.h"
#include "memory.h"
#include "platform.h"
#include "renderer.h"

Render_state render_state;

i32 renderer_init(struct Render_state* renderer, i32 width, i32 height) {
  image_init(width, height, 16, &renderer->framebuffer);
  return NoError;
}

void renderer_swap_buffers(struct Render_state* renderer) {
  image_clear(&renderer->framebuffer, ColorRGB(255, 0, 0));
  platform_swap_buffers(renderer);
}

void renderer_free(struct Render_state* renderer) {
  image_free(&renderer->framebuffer);
}
