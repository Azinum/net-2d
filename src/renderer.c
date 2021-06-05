// renderer.c

#include "common.h"
#include "memory.h"
#include "platform.h"
#include "renderer.h"

Render_state render_state = {};
Render_state* renderer = &render_state;
Image shroom = {0};

static void switch_fb_target(Render_state* renderer);

// Double buffering
void switch_fb_target(Render_state* renderer) {
  renderer->framebuffer = &renderer->fb[renderer->fb_target];
  renderer->fb_target = !renderer->fb_target;
}

i32 renderer_init(struct Render_state* renderer, i32 width, i32 height) {
  renderer->fb_target = 1;
  image_init(width, height, 4, &renderer->fb[0]);
  image_init(width, height, 4, &renderer->fb[1]);
  switch_fb_target(renderer);
  image_load("resource/sprite/shroom.bmp", &shroom);
  return NoError;
}

void renderer_swap_buffers(struct Render_state* renderer) {
  switch_fb_target(renderer);
  platform_swap_buffers(renderer);
  image_clear(renderer->framebuffer, ColorRGB(255, 255, 255));
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

void render_quad_border(const v2 p, const v2 size, Color_rgba color) {
  Image* framebuffer = renderer->framebuffer;

  i32 steps = 0;
  i32 w = (i32)size.w;
  i32 h = (i32)size.h;
  for (i32 y = 0; y < h; ++y) {
    for (i32 x = 0; x < w; ++x) {
      steps++;
      if (x > 0 && y > 0 && x < w - 1 && y < h - 1) {
        x = w - 2;
        continue;
      }
      Color_rgba* pixel = image_grab_pixel(framebuffer, x + p.x, y + p.y);
      if (pixel) {
        *pixel = color;
      }
    }
  }
  (void)steps;
}

void render_image(Image* image, const v2 p, const v2 size, Color_rgba tint) {
  Image* framebuffer = renderer->framebuffer;

  // Mask to eliminate purple "transparent" pixels
  const u32 elimination_mask = (0xff << 0) | (0x0 << 8) | (0xff << 16) | (0x0 << 24);

  // TODO(lucas): Use screen boundaries to limit the area of pixels that we are iterating over

  v2 uv = V2(0, 0);
  i32 w = (i32)size.w;
  i32 h = (i32)size.h;

  for (i32 y = 0; y < h; ++y) {
    uv.y = (float)y / h;
    for (i32 x = 0; x < w; ++x) {
      uv.x = (float)x / w;
      i32 x_sample = ((i32)(uv.x * image->width) % image->width);
      i32 y_sample = ((i32)(uv.y * image->height) % image->height);
      Color_rgba* pixel = image_grab_pixel(framebuffer, x + p.x, y + p.y);
      Color_rgba* texel = image_grab_pixel(image, x_sample, y_sample);

      if (pixel && texel) {
        if (texel->value != elimination_mask) {
          *pixel = *texel;
        }
      }
    }
  }
}

void render_texture(u32 texture_id, const v2 p, const v2 size, Color_rgba tint) {
  render_image(&shroom, p, size, tint);
}

void renderer_free(struct Render_state* renderer) {
  image_free(&renderer->fb[0]);
  image_free(&renderer->fb[1]);
  image_free(&shroom);
}
