// image.h

#ifndef _IMAGE_H
#define _IMAGE_H

typedef struct Image {
  u8* data;
  i32 width;
  i32 height;
  u16 depth;
  u16 pitch;
  u16 bytes_per_pixel;
} Image;

typedef union Color_rgba {
  struct {
    u8 b;
    u8 g;
    u8 r;
    u8 a;
  };
  struct {
    u32 value;
  };
} Color_rgba;

#define ColorRGBA(R, G, B, A) (Color_rgba) {{ .r = R, .g = G, .b = B, .a = A, }}
#define ColorRGB(R, G, B) (Color_rgba) {{ .r = R, .g = G, .b = B, .a = 255, }}

i32 image_init(i32 width, i32 height, u16 bytes_per_pixel, Image* image);

i32 image_load(const char* path, Image* image);

Color_rgba* image_grab_pixel(Image* image, i32 x, i32 y);

void image_clear(Image* image, Color_rgba color);

void image_free(Image* image);

#endif
