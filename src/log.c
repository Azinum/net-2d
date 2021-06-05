// log.c

#include <stdarg.h>

#include "log.h"

static FILE* log_fp = NULL;

void log_init(FILE* fp) {
  log_fp = fp;
}

void log_printf(const char* format, ...) {
  if (!log_fp) {
    log_fp = stdout;
  }
{
  va_list args;
  va_start(args, format);
  vfprintf(log_fp, format, args);
  va_end(args);
}
#if 1
{
  fprintf(stdout, "LOG: ");
  va_list args;
  va_start(args, format);
  vfprintf(stdout, format, args);
  va_end(args);
}
#endif
}
