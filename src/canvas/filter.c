#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "canvas/canvas.h"


filter_t *
filter_create_gaussian(uint32_t width, uint32_t height, double sigma) {
  int x, y;
  filter_t *flt_ptr;


  flt_ptr = (filter_t *)calloc(1, sizeof(filter_t));
  if(flt_ptr == NULL) { return NULL; }

  flt_ptr->width  = width;
  flt_ptr->height = height;
  flt_ptr->sigma  = sigma;
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
