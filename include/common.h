// common.h

#ifndef _COMMON_H
#define _COMMON_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

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

#if 1
  #define errprintf(Format, ...) fprintf(stderr, "%s:%s:%i: " Format, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
  #define errprintf(Format, ...) fprintf(stderr, Format, ##__VA_ARGS__)
#endif

#endif
