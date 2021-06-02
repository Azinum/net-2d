// image.c

#include "common.h"
#include "memory.h"
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

void image_clear(Image* image, Color_rgba color) {
  memset(image->data, 0, image->bytes_per_pixel * image->width * image->height * sizeof(u8));
}

void image_free(Image* image) {
  if (image->data) {
    m_free(image->data, image->bytes_per_pixel * image->width * image->height * sizeof(u8));
  }
  memset(image, 0, sizeof(Image));
}
