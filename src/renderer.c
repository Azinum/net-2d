// renderer.c

#include "common.h"
#include "memory.h"
#include "platform.h"
#include "renderer.h"

Render_state render_state = {};
Render_state* renderer = &render_state;
Image spritesheet = {0};

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
  image_load("resource/sprite/spritesheet.bmp", &spritesheet);
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

void render_image(Image* image, const v2 p, const v2 size, const v2 uv_offset, const v2 uv_range, Color_rgba tint) {
  Image* framebuffer = renderer->framebuffer;

  // Mask to eliminate purple "transparent" pixels
  const u32 elimination_mask = (0xff << 0) | (0x0 << 8) | (0xff << 16) | (0x0 << 24);

  v2 uv = V2(0, 0);
  i32 w = (i32)size.w;
  i32 h = (i32)size.h;

  for (i32 y = 0; y < h; ++y) {
    uv.y = ((float)y / h) * uv_range.y + uv_offset.y;
    for (i32 x = 0; x < w; ++x) {
      uv.x = ((float)x / w) * uv_range.x + uv_offset.x;
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

void render_sprite(u32 spritesheet_id, u32 sprite_id, const v2 p, const v2 size, Color_rgba tint) {
  Image* texture = &spritesheet;
#define SPRITE_WIDTH 16
#define SPRITE_HEIGHT 16
  // TODO(lucas): Handle "multi-line" spritesheet textures
  float w_factor = (float)SPRITE_WIDTH / texture->width;
  float h_factor = (float)SPRITE_HEIGHT / texture->height;
  v2 uv_offset = V2(w_factor * sprite_id, 0);
  v2 uv_range = V2(w_factor, h_factor);
  render_image(texture, p, size, uv_offset, uv_range, tint);
}

void renderer_free(struct Render_state* renderer) {
  image_free(&renderer->fb[0]);
  image_free(&renderer->fb[1]);
  image_free(&spritesheet);
}
