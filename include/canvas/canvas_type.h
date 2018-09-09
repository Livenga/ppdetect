#ifndef _CANVAS_TYPE_H
#define _CANVAS_TYPE_H

#include <stdint.h>



typedef enum {
  GRAY = 1,
  GRAY_ALPHA,
  RGB,
  RGB_ALPHA
} color_type_t;

#define GA   GRAY_ALPHA
#define RGBA RGB_ALPHA


typedef unsigned char uchar_t;

typedef struct {
  uint8_t r, g, b;
} color_t;


typedef struct {
  uint32_t width;
  uint32_t height;
  color_type_t  color_type;

  void *data;
} canvas_common_t;


typedef struct {
  uint32_t width;
  uint32_t height;

  color_type_t color_type;

  uchar_t *data;
} canvas_t;


typedef struct {
  uint32_t width;
  uint32_t height;

  color_type_t color_type;

  double *data;
} ncanvas_t;

#endif
