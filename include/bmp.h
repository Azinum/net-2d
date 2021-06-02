// bmp.h

#ifndef _BMP_H
#define _BMP_H

struct Image;

i32 bmp_load_from_path(const char* path, struct Image* image);

#endif
