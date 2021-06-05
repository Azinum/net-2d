// common.c

#include "memory.h"
#include "common.h"

void* list_initialize(const i32 size, const i32 count) {
	void* list = m_calloc(size, count);
	if (!list) {
    errprintf("Failed to allocate memory for list\n");
		return NULL;
	}
	return list;
}

i32 buffer_init(Buffer* buffer) {
  memset(buffer, 0, sizeof(Buffer));
  void* data = list_initialize(DEFAULT_BUFFER_SIZE, 1);
  if (data) {
    buffer->data = (u8*)data;
    buffer->count = 0;
    buffer->size = DEFAULT_BUFFER_SIZE;
    return NoError;
  }
  return Error;
}

i32 buffer_write_byte(Buffer* buffer, u8 byte) {
  if (buffer->count + 1 < buffer->size) {
    buffer->data[buffer->count++] = byte;
  }
  else {
    assert("buffer overflow" && 0);
    // Handle, resize buffer
  }
  return NoError;
}

i32 buffer_write_string(Buffer* buffer, const char* format, ...) {
  va_list args;
  va_start(args, format);
  buffer->count += vsnprintf((void*)buffer->data, buffer->size - buffer->count, format, args);
  va_end(args);
  return NoError;
}

i32 buffer_write_data(Buffer* buffer, void* data, i32 data_size) {
  if (buffer->count + data_size < buffer->size) {
    memcpy(&buffer->data[buffer->count], data, data_size);
    buffer->count += data_size;
  }
  else {
    assert("buffer overflow" && 0);
    // Handle, resize buffer
    return Error;
  }
  return NoError;
}

void buffer_print(FILE* fp, Buffer* buffer) {
  fprintf(fp, "%.*s\n", buffer->count, buffer->data);
}

void buffer_free(Buffer* buffer) {
  list_free(buffer->data, buffer->size);
  memset(buffer, 0, sizeof(Buffer));
}

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

u8* write_data(u8* buffer, i32 buffer_size, void* data, i32 data_size) {
  if (data_size <= buffer_size) {
    memcpy(buffer, data, data_size);  // TODO(lucas): Implement SIMD variant of memcpy
    return buffer + data_size;
  }
  return NULL;
}
