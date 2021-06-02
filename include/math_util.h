// math_util.h

#ifndef _MATH_UTIL_H
#define _MATH_UTIL_H

#ifndef NO_SSE

#if __SSE__
  #define USE_SSE 1
#endif

#endif

#if USE_SSE
  #include <xmmintrin.h>
#endif

typedef union v2 {
  struct {
    i32 x;
    i32 y;
  };
} v2;

#define V2(X, Y) (v2) {{ .x = X, .y = Y, }}

#endif
