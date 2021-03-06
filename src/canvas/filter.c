#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "canvas/canvas.h"

#define _FLT_TRUE  ((uint8_t)1)
#define _FLT_FALSE ((uint8_t)0)


filter_t *
filter_create_gaussian(uint32_t width, uint32_t height, double sigma) {
  int x, y;
  filter_t *flt_ptr;


  flt_ptr = (filter_t *)calloc(1, sizeof(filter_t));
  if(flt_ptr == NULL) { return NULL; }

  flt_ptr->width  = width;
  flt_ptr->height = height;
  /*flt_ptr->sigma  = sigma;*/
  flt_ptr->data   = (double *)calloc(width * height, sizeof(double));

  if(flt_ptr->data == NULL) {
    memset((void *)flt_ptr, '\0', sizeof(filter_t));
    free((void *)flt_ptr); flt_ptr = NULL;
    return NULL;
  }


  for(y = 0; y < height; ++y) {
    const int _y = y - (height / 2);

    for(x = 0; x < width; ++x) {
      const int _x = x - (width / 2);

      *(flt_ptr->data + (y * width + x)) =
        (1.0 / ((sqrt(2.0 * M_PI) * sigma)))
        * (exp(-(_x * _x + _y * _y) / (2.0 * pow(sigma, 2.0))));
    }
  }

  return flt_ptr;
}

static filter_t *
_filter_create_sobel(uint8_t is_x) {
  filter_t *fx_sobel;

  if((fx_sobel = (filter_t *)calloc(1, sizeof(filter_t))) != NULL) {
    fx_sobel->width  = 3;
    fx_sobel->height = 3;

    if((fx_sobel->data = (double *)calloc(9, sizeof(double))) != NULL) {
      if(is_x == _FLT_TRUE) {
        *(fx_sobel->data + 0) = -1.0;
        *(fx_sobel->data + 1) =  0.0;
        *(fx_sobel->data + 2) =  1.0;

        *(fx_sobel->data + 3) = -2.0;
        *(fx_sobel->data + 4) =  0.0;
        *(fx_sobel->data + 5) =  2.0;
        
        *(fx_sobel->data + 6) = -1.0;
        *(fx_sobel->data + 7) =  0.0;
        *(fx_sobel->data + 8) =  1.0;
      } else {
        *(fx_sobel->data + 0) = -1.0;
        *(fx_sobel->data + 1) = -2.0;
        *(fx_sobel->data + 2) = -1.0;

        *(fx_sobel->data + 3) =  0.0;
        *(fx_sobel->data + 4) =  0.0;
        *(fx_sobel->data + 5) =  0.0;
        
        *(fx_sobel->data + 6) =  1.0;
        *(fx_sobel->data + 7) =  2.0;
        *(fx_sobel->data + 8) =  1.0;
      }
    } else {
      memset((void *)fx_sobel, '\0', sizeof(filter_t));
      free((void *)fx_sobel);
      fx_sobel = NULL;
    }
  }

  return fx_sobel;
}

filter_t *
filter_create_sobel_x(void) {
  return _filter_create_sobel(/*is_x = */_FLT_TRUE);
}


filter_t *
filter_create_sobel_y(void) {
  return _filter_create_sobel(/*is_x*/_FLT_FALSE);
}

double
filter_convolution_partial(
    const ncanvas_t *self,
    uint32_t        pos_x,
    uint32_t        pos_y,
    const filter_t  *filter,
    double          outside) {
  int i, j;
  double sum;

  sum = 0.0;
  for(i = 0; i < filter->height; ++i) {
    const uint32_t _y = pos_y + (i - (filter->height / 2));

    for(j = 0; j < filter->width; ++j) {
      const uint32_t _x = pos_x + (j - (filter->width / 2));

      // 対象となる画像データ(ncanvas_t->data)が範囲外の場合, 
      // 引数 outside の数値を使用する.
      if(_x >= self->width || _x < 0 ||
          _y >= self->height || _y < 0) {
        sum += *(filter->data + (i * filter->width + j)) * outside;
      } else {
        sum += *(filter->data + (i * filter->width + j)) *
          *(self->data + (_y * self->width + _x));
      }
    }
  }

  return sum;
}

ncanvas_t *
filter_convolution(ncanvas_t *ncv_ptr, const filter_t *filter) {
  int i, j;
  ncanvas_t *ret_ptr;


  ret_ptr = ncv_alloc(ncv_ptr->width, ncv_ptr->height, GRAY);
  if(ret_ptr == NULL) {
    return NULL;
  }

  for(i = 0; i < ncv_ptr->height; ++i) {
    for(j = 0; j < ncv_ptr->width; ++j) {
      double _conv;

      _conv = filter_convolution_partial(ncv_ptr, j, i, filter, 0.0);
#if 1
      *(ret_ptr->data + (i * ncv_ptr->width + j)) =
        (_conv > 1.0) ? 1.0 : (_conv < 0.0) ? 0.0 : _conv;
#else
      *(ret_ptr->data + (i * ncv_ptr->width + j)) = _conv;
#endif
    }
  }

  return ret_ptr;
}


/**
*/
void
filter_free(filter_t *flt_ptr) {
  if(flt_ptr == NULL) { return; }

  if(flt_ptr->data != NULL) {
    memset((void *)flt_ptr->data, '\0', PFILTER_SIZE(flt_ptr));
    free((void *)flt_ptr->data); flt_ptr->data = NULL;
  }

  memset((void *)flt_ptr, '\0', sizeof(filter_t));
  free((void *)flt_ptr); flt_ptr = NULL;
}
