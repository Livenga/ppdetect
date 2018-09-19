#ifndef _PPD_TYPE_H
#define _PPD_TYPE_H

#include <stdint.h>

typedef uint8_t bool_t;

#define TRUE  (bool_t)1
#define FALSE (bool_t)0


// 多数決構造
typedef struct _vote_point_t {
  uint32_t count;
  hough_point_t *point;
} vote_point_t;


typedef struct _score_t {
  int x, y;
  double score;
} score_t;

#endif
