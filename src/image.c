// image.c

#include "common.h"
#include "memory.h"
#include "bmp.h"
#include "image.h"

i32 image_init(i32 width, i32 height, u16 bytes_per_pixel, Image* image) {
  image->data = m_calloc(1, bytes_per_pixel * width * height * sizeof(u8));
  if (!image->data) {
    return Error;
  }
  image->width = width;
  image->height = height;
  image->depth = sizeof(u8) * 8; // 8 bits for each color component in any given pixel
  image->pitch = width * bytes_per_pixel;
  image->bytes_per_pixel = bytes_per_pixel;
  return NoError;
}

void image_print_info(FILE* fp, Image* image) {
  fprintf(fp,
    "data:            %p\n"
    "width:           %i\n"
    "height:          %i\n"
    "depth:           %i\n"
    "pitch:           %i\n"
    "bytes_per_pixel: %i",
    image->data,
    image->width,
    image->height,
    image->depth,
    image->pitch,
    image->bytes_per_pixel
  );
}

i32 image_load(const char* path, Image* image) {
  char* ext = get_extension(path);
  if (!strcmp(ext, ".bmp")) {
    return bmp_load_from_path(path, image);
  }
  else {
    return Error;
  }
  return NoError;
}

Color_rgba* image_grab_pixel(Image* image, i32 x, i32 y) {
  Color_rgba* pixel = NULL;
  Color_rgba* pixels = (Color_rgba*)&image->data[0];
  if (x < 0 || y < 0 || x >= image->width || y >= image->height) {
    return NULL;
  }

  pixel = &pixels[(image->width * y) + x];

  return pixel;
}

void image_clear(Image* image, Color_rgba color) {
  i32 size = (image->bytes_per_pixel * image->width * image->height * sizeof(u8));
#if USE_SSE
  __m128i* dest = (__m128i*)&image->data[0];
  __m128i value = _mm_setr_epi8(
    color.b, color.g, color.r, color.a,
    color.b, color.g, color.r, color.a,
    color.b, color.g, color.r, color.a,
    color.b, color.g, color.r, color.a
  );
  i32 chunk_size = sizeof(u8) * 4 * 4;
  i32 chunk_count = size / chunk_size;
  for (i32 i = 0; i < chunk_count; ++i, ++dest) {
    *dest = value;
  }
#else
  Color_rgba* dest = (Color_rgba*)&image->data[0];

  for (u32 i = 0; i < size / sizeof(Color_rgba); ++i, ++dest) {
    *dest = color;
  }
#endif
}

void image_free(Image* image) {
  if (image->data) {
    m_free(image->data, image->bytes_per_pixel * image->width * image->height * sizeof(u8));
  }
  memset(image, 0, sizeof(Image));
}
