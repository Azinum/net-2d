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
