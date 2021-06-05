// common.h

#ifndef _COMMON_H
#define _COMMON_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/time.h>

#define PI32 3.14159265359f
#define MAX_PATH_SIZE 512
#define ARRAY_SIZE(ARR) ((sizeof(ARR)) / (sizeof(ARR[0])))

enum Status_code {
  Error = -1,
  NoError = 0,
  Warning,
};

typedef int64_t i64;
typedef uint64_t u64;
typedef int32_t i32;
typedef uint32_t u32;
typedef int16_t i16;
typedef uint16_t u16;
typedef int8_t i8;
typedef uint8_t u8;
typedef float r32;
typedef double r64;

typedef union v2 {
  struct {
    float x, y;
  };
  struct {
    float w, h;
  };
} v2;

#define V2(X, Y) (v2) {{ .x = X, .y = Y, }}

#if 1
  #define errprintf(Format, ...) fprintf(stderr, "%s:%s:%i: " Format, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
  #define errprintf(Format, ...) fprintf(stderr, Format, ##__VA_ARGS__)
#endif

#define list_push(List, Count, Element) do { \
	if (List == NULL) { \
    List = (typeof(Element)*)list_initialize(sizeof(Element), 1); List[0] = Element; Count = 1; break; \
  } \
	void* NewList = m_realloc(List, Count * sizeof(*List), (1 + Count) * (sizeof(Element))); \
	if (NewList) { \
		List = (typeof(Element)*)NewList; \
		List[(Count)++] = Element; \
	} \
} while (0); \

#define list_realloc(List, Count, NewSize) do { \
  if (List == NULL) break; \
  if (NewSize == 0) { list_free(List, Count); break; } \
	void* NewList = m_realloc(List, Count * sizeof(*List), (NewSize) * (sizeof(*List))); \
  List = NewList; \
  Count = NewSize; \
} while(0); \

#define list_shrink(List, Count, Num) { \
  if ((Count - Num) >= 0) { \
	  list_realloc(List, Count, Count - Num); \
  } \
}

#define list_assign(List, Count, Index, Element) { \
	assert(List != NULL); \
	if (Index < Count) { \
		List[Index] = Element; \
	} else { \
		assert(0); \
	} \
} \

#define list_free(List, Count) { \
	if ((List) != NULL && Count > 0) { \
		m_free(List, Count * sizeof(*List)); \
		Count = 0; \
		List = NULL; \
	}\
}

#define DEFAULT_BUFFER_SIZE 4096

typedef struct Buffer {
  u8* data;
  u32 count;  // The count bytes in this buffer that is filled
  u32 size; // Size allocated
} Buffer;

void* list_initialize(const i32 size, const i32 count);

i32 buffer_init(Buffer* buffer);

i32 buffer_write_byte(Buffer* buffer, u8 byte);

i32 buffer_write_string(Buffer* buffer, const char* format, ...);

i32 buffer_write_data(Buffer* buffer, void* data, i32 data_size);

void buffer_print(FILE* fp, Buffer* buffer);

void buffer_free(Buffer* buffer);

char* get_extension(const char* path);

u8* write_byte(u8* buffer, u8 byte);

u8* write_string(u8* buffer, i32 buffer_size, const char* format, ...);

u8* write_data(u8* buffer, i32 buffer_size, void* data, i32 data_size);

#endif
