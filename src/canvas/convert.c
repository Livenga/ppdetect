#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "canvas/canvas.h"

#define _IS_ROUNDING

static uint32_t 
_calc_rounding32(const double value) {
  // Decimal point
  double dp = value - (int32_t)value;
  return (int32_t)value + (dp >=5.0 ? 1 : 0);
}


canvas_t *
cv_grayscale(const canvas_t *cv_cptr) {
  canvas_t *gray_ptr;


  if(cv_cptr == NULL ||
      cv_cptr->color_type == GRAY || cv_cptr->color_type == GA) {
    return NULL;
  }

  gray_ptr = cv_alloc(cv_cptr->width, cv_cptr->height, GRAY);
  if(gray_ptr == NULL) { return NULL; }


  int i, j;
  for(i = 0; i < cv_cptr->height; ++i) {
    for(j = 0; j < cv_cptr->width; ++j) {
      const size_t _offset = ((i * cv_cptr->width) + j);
      double _g;

      _g =  (0.2126 * cv_cptr->data[_offset * cv_cptr->color_type + 0]) +
      (0.7152 * cv_cptr->data[_offset * cv_cptr->color_type + 1]) +
      (0.0722 * cv_cptr->data[_offset * cv_cptr->color_type + 2]);

      gray_ptr->data[_offset] =
#ifdef _IS_ROUNDING
        ((_g > 255.0) ? 0xFF : ((_g < 0.0) ? 0x00 : (uchar_t)_calc_rounding32(_g)));
#else
        ((_g > 255.0) ? 0xFF : ((_g < 0.0) ? 0x00 : (uchar_t)_g));
#endif
    }
  }

  return gray_ptr;
}
