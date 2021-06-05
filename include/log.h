// log.h

#ifndef _LOG_H
#define _LOG_H

#include "common.h"

void log_init(FILE* fp);

void log_printf(const char* format, ...);

#endif
