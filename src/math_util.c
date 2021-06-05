// math_util.c

#include "common.h"
#include "math_util.h"

inline float lerp(float v0, float v1, float t) {
  return (1.0f - t) * v0 + t * v1;
}
