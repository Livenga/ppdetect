#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "canvas/canvas.h"

//#define _ADD_PTR 1

ncanvas_t *
cv2ncv(const canvas_t *cv_cptr) {
  ncanvas_t *ncv_ptr;



  ncv_ptr = ncv_alloc(cv_cptr->width, cv_cptr->height, cv_cptr->color_type);
  if(ncv_ptr != NULL) {
    int i, j, k;

    const uint8_t data_size = cv_get_data_size((canvas_common_t *)cv_cptr);

    for(i = 0; i < ncv_ptr->height; ++i) {
      for(j = 0; j < ncv_ptr->width; ++j) {
        for(k = 0; k < data_size; ++k) {
          size_t _offset = (i * ncv_ptr->width + j) * data_size + k;

          ncv_ptr->data[_offset] = (double)cv_cptr->data[_offset] / 255.0;
        } // for(k)
      } // for(j)
    } // for(i)
  }

  return ncv_ptr;
}


canvas_t *
ncv2cv(const ncanvas_t *ncv_cptr) {
  canvas_t *cv_ptr;


  if((cv_ptr = cv_alloc(ncv_cptr->width, ncv_cptr->height, ncv_cptr->color_type)) != NULL) {
    int i, j, k;
  
    const uint8_t data_size = cv_get_data_size((canvas_common_t *)ncv_cptr);

    for(i = 0; i < cv_ptr->height; ++i) {
      for(j = 0; j < cv_ptr->width; ++j) {
        for(k = 0; k < data_size; ++k) {
          double _value;
          size_t _offset = (i * cv_ptr->width + j) * data_size + k;

          _value = ncv_cptr->data[_offset] * 255.0;

          cv_ptr->data[_offset] =
            (_value > 255.0) ? 0xFF :
            (_value <   0.0) ? 0x00 :
            (uchar_t)_value;
        } // for(k)
      } // for(j)
    } // for(i)
  }

  return cv_ptr;
}
