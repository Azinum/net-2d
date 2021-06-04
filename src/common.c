// common.c

#include "common.h"

char* get_extension(const char* path) {
  char* iter = (char*)path;
  if (!iter)
    return NULL;
  char ch = 0;
  while ((ch = *iter) != '\0') {
    if (ch == '.') {
      return iter;
    }
    iter++;
  }
  return iter;
}

u8* write_byte(u8* buffer, u8 byte) {
  *buffer = byte;
  return buffer + 1;
}

u8* write_string(u8* buffer, i32 buffer_size, const char* format, ...) {
  i32 size = 0;
  va_list args;
  va_start(args, format);
  size = vsnprintf((void*)buffer, buffer_size, format, args);
  va_end(args);
  return buffer + size;
}
