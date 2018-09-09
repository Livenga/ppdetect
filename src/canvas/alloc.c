#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "canvas/canvas.h"


static uint8_t
_cv_get_data_size(color_type_t color_type) {
  switch(color_type) {
    case GRAY: return 1;
    case GA:   return 2;
    case RGB:  return 3;
    case RGBA: return 4;
  }

  return 255;
}

uint8_t
cv_get_data_size(canvas_common_t *cv_ptr) {
  return _cv_get_data_size(cv_ptr->color_type);
}

uint8_t
cv_get_data_size_v(color_type_t color_type) {
  return _cv_get_data_size(color_type);
}


canvas_t *
cv_alloc(uint32_t width, uint32_t height, color_type_t color_type) {
  canvas_t *cv_ptr;


  cv_ptr = (canvas_t *)calloc(1, sizeof(canvas_t));
  if(cv_ptr == NULL) {
    return NULL;
  }

  cv_ptr->width      = width;
  cv_ptr->height     = height;
  cv_ptr->color_type = color_type;


  cv_ptr->data = (uchar_t *)calloc(PCANVAS_SIZE(cv_ptr), sizeof(uchar_t));
  if(cv_ptr->data == NULL) {
    memset((void *)cv_ptr, '\0', sizeof(canvas_t));
    free((void *)cv_ptr);
    cv_ptr = NULL;
  }

  return cv_ptr;
}

ncanvas_t *
ncv_alloc(uint32_t width, uint32_t height, color_type_t color_type) {
  ncanvas_t *ncv_ptr;


  if((ncv_ptr = (ncanvas_t *)calloc(1, sizeof(ncanvas_t))) == NULL) {
    return NULL;
  }

  ncv_ptr->width      = width;
  ncv_ptr->height     = height;
  ncv_ptr->color_type = color_type;

  ncv_ptr->data = (double *)calloc(PCANVAS_SIZE(ncv_ptr), sizeof(double));
  if(ncv_ptr->data == NULL) {
    memset((void *)ncv_ptr, '\0', sizeof(ncanvas_t));

    free((void *)ncv_ptr);
    ncv_ptr = NULL;
  }

  return ncv_ptr;
}


static void
_cvc_common_free(canvas_common_t *cvc_ptr, size_t data_size) {
  if(cvc_ptr == NULL) { return; }

  if(cvc_ptr->data != NULL) {
    memset((void *)cvc_ptr->data, '\0', data_size * PCANVAS_SIZE(cvc_ptr));

    free((void *)cvc_ptr->data);
    cvc_ptr->data = NULL;
  }

  memset((void *)cvc_ptr, '\0', sizeof(canvas_common_t));

  free((void *)cvc_ptr);
  cvc_ptr = NULL;
}


void
cv_free(canvas_t *cv_ptr) {
  _cvc_common_free((canvas_common_t *)cv_ptr, sizeof(uchar_t));
}


void
ncv_free(ncanvas_t *ncv_ptr) {
  _cvc_common_free((canvas_common_t *)ncv_ptr, sizeof(double));
}
