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

#endif
