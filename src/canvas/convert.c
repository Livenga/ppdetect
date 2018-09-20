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

// 線形補間変換
canvas_t *
cv_bilinear(const canvas_t *cv_cptr, uint32_t width, uint32_t height) {
  const double dx = (double)width / cv_cptr->width,
         dy = (double)height / cv_cptr->height;
  int i, j, k;
  canvas_t *ret_cptr;


  if((ret_cptr = cv_alloc(width, height, cv_cptr->color_type)) == NULL) {
    return NULL;
  }

  fprintf(stderr, "* 線形補間 倍率(X: %.2f, Y: %.2f)  (%ux%u) -> (%ux%u)\n",
      dx, dy,
      cv_cptr->width, cv_cptr->height,
      width, height);


  uchar_t pixel[4];
  double p, q;
  double linear_x0, linear_x1, linear_y;

  for(i = 0; i < height; ++i) {
    const double   base_y = i / dy;
    const uint32_t _y = (uint32_t)base_y;

    p = base_y - _y;
    for(j = 0; j < width; ++j) {
      const double base_x = j / dx;
      const uint32_t _x   = (uint32_t)base_x;
      q = base_x - _x;

      for(k = 0; k < cv_cptr->color_type; ++k) {
        pixel[0] = *(cv_cptr->data + (((_y + 0) * cv_cptr->width + (_x + 0)) * cv_cptr->color_type + k));
        pixel[1] = *(cv_cptr->data + (((_y + 0) * cv_cptr->width + (_x + 1)) * cv_cptr->color_type + k));
        pixel[2] = *(cv_cptr->data + (((_y + 1) * cv_cptr->width + (_x + 0)) * cv_cptr->color_type + k));
        pixel[3] = *(cv_cptr->data + (((_y + 1) * cv_cptr->width + (_x + 1)) * cv_cptr->color_type + k));

        linear_x0 = pixel[1]  * (1.0 - q) + pixel[0] * q;
        linear_x1 = pixel[3]  * (1.0 - q) + pixel[2] * q;
        linear_y  = linear_x1 * (1.0 - p) + linear_x0 * p;

        *(ret_cptr->data + ((i * width + j) * cv_cptr->color_type + k)) =
          (linear_y > 255.0) ? 0xFF : (linear_y < 0.0) ? 0x00 : (uchar_t)linear_y;
      }
    }
  }

  return ret_cptr;
}


canvas_t *
cv_bilinear_magnif(const canvas_t *cv_cptr, const double magnif) {
  return cv_bilinear(cv_cptr,
      (uint32_t)(cv_cptr->width  * magnif),
      (uint32_t)(cv_cptr->height * magnif));
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
