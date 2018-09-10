#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "canvas/canvas.h"

#define _IS_ROUNDING


#ifdef _IS_ROUNDING
static uint32_t 
_calc_rounding32(const double value) {
  // Decimal point
  double dp = value - (int32_t)value;
  return (int32_t)value + (dp >=5.0 ? 1 : 0);
}
#endif


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

ncanvas_t *
ncv_grayscale(const ncanvas_t *ncv_cptr) {
  ncanvas_t *gray_ptr;


  if(ncv_cptr == NULL || ncv_cptr->data == NULL) { return NULL; }
  else if(ncv_cptr->color_type == GRAY || ncv_cptr->color_type == GA) { return NULL; }


  if((gray_ptr = ncv_alloc(ncv_cptr->width, ncv_cptr->height, GRAY)) != NULL) {
    int i, j;

    for(i = 0; i < ncv_cptr->height; ++i) {
      for(j = 0; j < ncv_cptr->width; ++j) {
        const uint32_t _offset = (i * ncv_cptr->width + j);
        double _g;

        _g = (0.2126 * *(ncv_cptr->data + (_offset * ncv_cptr->color_type + 0)))
          + (0.7152 * *(ncv_cptr->data + (_offset * ncv_cptr->color_type + 1)))
          + (0.0722 * *(ncv_cptr->data + (_offset * ncv_cptr->color_type + 2)));

        *(gray_ptr->data + _offset) = (_g > 1.0) ? 1.0 :
          ((_g < 0.0) ? 0.0 : _g);
      }
    }
  }


  return gray_ptr;
}


void
ncv_inverse(ncanvas_t *ncv_ptr) {
  int i, j, k;


  for(i = 0; i < ncv_ptr->height; ++i) {
    for(j = 0; j < ncv_ptr->width; ++j) {
      for(k = 0; k < ncv_ptr->color_type; ++k) {
        const size_t _offset = (i * ncv_ptr->width + j) * ncv_ptr->color_type + k;
        *(ncv_ptr->data + _offset) = 1.0 - *(ncv_ptr->data + _offset);
      }
    }
  }
}

void
ncv_binarize(ncanvas_t *ncv_ptr, double threshold) {
  int i, j;


  if(ncv_ptr->color_type != GRAY) { return; }

  for(i = 0; i < ncv_ptr->height; ++i) {
    for(j = 0; j < ncv_ptr->width; ++j) {
      const size_t _offset = i * ncv_ptr->width + j;

      *(ncv_ptr->data + _offset) = (*(ncv_ptr->data + _offset) > threshold) ? 1.0 : 0.0;
    }
  }
}
