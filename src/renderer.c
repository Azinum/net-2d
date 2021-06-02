// renderer.c

#include "common.h"
#include "memory.h"
#include "platform.h"
#include "renderer.h"

Render_state render_state = {};
Render_state* renderer = &render_state;

static void switch_fb_target(Render_state* renderer);

void switch_fb_target(Render_state* renderer) {
  renderer->fb_target = !renderer->fb_target;
  renderer->framebuffer = &renderer->fb[renderer->fb_target];
}

i32 renderer_init(struct Render_state* renderer, i32 width, i32 height) {
  renderer->fb_target = 1;
  image_init(width, height, 4, &renderer->fb[0]);
  image_init(width, height, 4, &renderer->fb[1]);
  switch_fb_target(renderer);
  return NoError;
}

void renderer_swap_buffers(struct Render_state* renderer) {
  switch_fb_target(renderer);
  platform_swap_buffers(renderer);
  image_clear(renderer->framebuffer, ColorRGB(0, 0, 0));
}

void render_quad(const v2 p, const v2 size, Color_rgba color) {
  Image* framebuffer = renderer->framebuffer;

  for (i32 y = p.y; y < p.y + size.y; ++y) {
    for (i32 x = p.x; x < p.x + size.x; ++x) {
      Color_rgba* pixel = image_grab_pixel(framebuffer, x, y);
      if (pixel) {
        *pixel = color;
      }
    }
  }
}

void render_image(Image* image, const v2 p, const v2 size, Color_rgba tint) {
  Image* framebuffer = renderer->framebuffer;

  const u32 EliminationMask = (0xff << 0) | (0x0 << 8) | (0xff << 16) | (0x0 << 24);  // Mask to eliminate purple "transparent" pixels

  for (i32 y = 0; y < image->height; ++y) {
    for (i32 x = 0; x < image->width; ++x) {
      Color_rgba* pixel = image_grab_pixel(framebuffer, p.x + x, p.y + y);
      Color_rgba* color = image_grab_pixel(image, x, y);
      if (pixel && color) {
        if (color->value != EliminationMask) {
          *pixel = *color;
        }
      }
    }
  }
}

void renderer_free(struct Render_state* renderer) {
  image_free(&renderer->fb[0]);
  image_free(&renderer->fb[1]);
}
