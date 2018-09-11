#ifndef _CALC_TYPE_H
#define _CALC_TYPE_H

#include <stdint.h>


#define NUMBER_OF_HOUGH_POINT 181


typedef enum {
  nR = 0, nG, nB
} cc_id_t;


typedef struct {
  uint8_t id;
  double value[3];
} cluster_t;


typedef struct {
  uint32_t count;
  double value[3];
} gcenter_t;


typedef struct _harris_point_t harris_point_t;
typedef struct _harris_point_t {
  uint32_t x, y;
  double   rate;

  harris_point_t *next;
} harris_point_t;


typedef struct _hough_point_t hough_point_t;
struct _hough_point_t {
  uint32_t x;
  uint32_t y;

  double radian;
  double rho;

  hough_point_t *next;
};
#endif
