#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "ppd_type.h"
#include "calc/calc_type.h"
#include "canvas/canvas.h"
#include "util.h"


//#undef _ENABLE_CSV


hough_point_t *
hough_point_new(uint32_t x, uint32_t y,
    double radian, double rho) {
  hough_point_t *p;


  if((p = (hough_point_t *)calloc(1, sizeof(hough_point_t))) != NULL) {
    p->x      = x;
    p->y      = y;
    p->radian = radian;
    p->rho    = rho;

    p->next = NULL;
  }

  return p;
}

size_t
hough_point_count(const hough_point_t *hp) {
  const hough_point_t *hp_cur = hp;
  size_t count = 0;

  if(hp == NULL) { return 0; }

  for(; hp_cur != NULL; hp_cur = hp_cur->next) { ++count; }
  return count;
}

void
hough_point_release(hough_point_t *p) {
  hough_point_t *p_next;

  do {
    p_next = p->next;

    memset((void *)p, '\0', sizeof(hough_point_t));
    free((void *)p);

    p = p_next;
  } while(p != NULL);
}

void
hough_points_release(hough_point_t **hp_ptr) {
  int i;

  for(i = 0; i < NUMBER_OF_HOUGH_POINT; ++i) {
    hough_point_release(*(hp_ptr + i));
    *(hp_ptr + i) = NULL;
  }

  memset((void *)hp_ptr, '\0', sizeof(hough_point_t *) * NUMBER_OF_HOUGH_POINT);
  free((void *)hp_ptr);
  hp_ptr = NULL;
}



hough_point_t **
run_hough_transform(
    const harris_point_t *harris_points,
    const double          harris_threshold) {
  const harris_point_t  *harris_cursor;
  uint32_t theta_i;

  double rho;

  hough_point_t **hp_root, **hp_end;


  hp_root = (hough_point_t **)calloc(NUMBER_OF_HOUGH_POINT, sizeof(hough_point_t *));
  hp_end  = (hough_point_t **)calloc(NUMBER_OF_HOUGH_POINT, sizeof(hough_point_t *));


#ifdef _ENABLE_CSV
  char *csv_name;
  FILE *csv_fp;

  csv_name = get_output_filename("hough", NULL, "csv");
  csv_fp   = fopen(csv_name, "w");
#endif


  for(harris_cursor = harris_points;
      harris_cursor != NULL;
      harris_cursor = harris_cursor->next) {
    if(harris_cursor->rate >= harris_threshold) {
      for(theta_i = 0; theta_i < NUMBER_OF_HOUGH_POINT; ++theta_i) {
        const double _rad = (((double)theta_i - 90.0) * M_PI) / 180.0;
        hough_point_t *_hp;


        rho = harris_cursor->x * cos(_rad) + harris_cursor->y * sin(_rad);
        _hp = hough_point_new(harris_cursor->x, harris_cursor->y, _rad, rho);

#ifdef _ENABLE_CSV
        if(csv_fp != NULL) { fprintf(csv_fp, "%f %f\n", _rad, rho); }
#endif

        if(*(hp_root + theta_i) == NULL) {
          *(hp_root + theta_i) = _hp;
          *(hp_end  + theta_i) = _hp;
        }

        if(*(hp_end + theta_i) == NULL) {
          *(hp_end + theta_i) = _hp;
        } else {
          (*(hp_end + theta_i))->next = _hp;
          *(hp_end + theta_i) = _hp;
        }
      }
    }
  }

#ifdef _ENABLE_CSV
  if(csv_fp != NULL) { fclose(csv_fp); }
#endif


  memset((void *)hp_end, '\0', sizeof(hough_point_t *) * 181);
  free((void *)hp_end);
  hp_end = NULL;

  return hp_root;
}
