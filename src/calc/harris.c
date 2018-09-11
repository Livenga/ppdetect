#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "ppd_type.h"
#include "calc/calc_type.h"
#include "canvas/canvas.h"


#define _K (0.24)
//#define _PPD_OUTPUT_CSV


harris_point_t *
harris_point_new(uint32_t x, uint32_t y, double rate) {
  harris_point_t *p_ptr;

  if((p_ptr = (harris_point_t *)calloc(1, sizeof(harris_point_t))) != NULL) {
    p_ptr->x    = x;
    p_ptr->y    = y;
    p_ptr->rate = rate;
    p_ptr->next = NULL;
  }

  return p_ptr;
}


void
harris_point_release(harris_point_t *pt_ptr) {
  harris_point_t *pt_next;

  do {
    pt_next = pt_ptr->next;

    memset((void *)pt_ptr, '\0', sizeof(harris_point_t));
    free((void *)pt_ptr);

    pt_ptr = pt_next;
  } while(pt_ptr != NULL);
}

harris_point_t *
harris_corner_detector(const ncanvas_t *ncv_cptr, double *threshold) {
#ifdef _PPD_OUTPUT_CSV
  FILE *csv_fp;
#endif

  int i, j;
  filter_t *f_gauss, *fx_sobel, *fy_sobel;
  harris_point_t *pt_root_ptr, *pt_cursor_ptr;

  
  if(ncv_cptr == NULL || threshold == NULL) {
    return NULL;
  }

  pt_root_ptr   = NULL;
  pt_cursor_ptr = NULL;

  f_gauss  = filter_create_gaussian(3, 3, 3.0);
  fx_sobel = filter_create_sobel_x();
  fy_sobel = filter_create_sobel_y();

#ifdef _PPD_OUTPUT_CSV
  csv_fp = fopen("harris.csv", "w");
#endif

  for(i = 1; i < ncv_cptr->height - 1; ++i) {
    for(j = 1; j < ncv_cptr->width - 1; ++j) {
      double _w, _ix, _iy, _ixx, _ixy, /*_iyx, */ _iyy;
      double _b, _c, _l1, _l2, _rate, _abs_rate;

#if 1
      _w  = filter_convolution_partial(ncv_cptr, j, i, f_gauss, 0.0);
      _ix = filter_convolution_partial(ncv_cptr, j, i, fx_sobel, 0.0);
      _iy = filter_convolution_partial(ncv_cptr, j, i, fy_sobel, 0.0);
#else
      _w  = filter_convolution_partial(ncv_cptr, j, i, f_gauss, 1.0);
      _ix = filter_convolution_partial(ncv_cptr, j, i, fx_sobel, 1.0);
      _iy = filter_convolution_partial(ncv_cptr, j, i, fy_sobel, 1.0);
#endif

      _ixx = _w * (_ix * _ix);
      _ixy = _w * (_ix * _iy);
      _iyy = _w * (_iy * _iy);

      _b = -(_ixx + _iyy);
      _c = (_ixx * _iyy) - (_ixy * _ixy);

      _l1 = (-_b + sqrt(_b * _b - 4.0 * _c)) / 2.0;
      _l2 = (-_b - sqrt(_b * _b - 4.0 * _c)) / 2.0;

      _rate     = (_l1 * _l2) - (_K * ((_l1 + _l2) * (_l1 + _l2)));
      _abs_rate = (_rate > 0.0) ? _rate : -_rate;

      if(_abs_rate != 1.0) {
        if(pt_root_ptr == NULL) {
          pt_root_ptr = harris_point_new(j, i, _abs_rate);
          pt_cursor_ptr = pt_root_ptr;
        } else {
          pt_cursor_ptr->next = harris_point_new(j, i, _abs_rate);
          pt_cursor_ptr = pt_cursor_ptr->next;
        }
      }
#ifdef _PPD_OUTPUT_CSV
      if(csv_fp != NULL) {
        fprintf(csv_fp, "%f\n", _abs_rate);
      }
#endif
    }
  }

  double max_rate = 0.0;
  harris_point_t *hp_p;

  for(hp_p = pt_root_ptr; hp_p != NULL; hp_p = hp_p->next) {
    if(max_rate < hp_p->rate) {
      max_rate = hp_p->rate;
    }
  }
  *threshold = max_rate - (max_rate / 10.0);

  fprintf(stderr, "* Max rate      : %f\n", max_rate);
  fprintf(stderr, "* Rate threshold: %f\n", *threshold);


  filter_free(fy_sobel);
  filter_free(fx_sobel);
  filter_free(f_gauss);

#ifdef _PPD_OUTPUT_CSV
  if(csv_fp != NULL) { fclose(csv_fp); }
#endif

  return pt_root_ptr;
}
