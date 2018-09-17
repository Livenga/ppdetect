#ifndef _TEMP_H
#define _TEMP_H

typedef struct _vector_t {
  double x, y;
} _vector_t;

typedef struct {
  _vector_t start;
  _vector_t end;
} _vector_line_t;


// 多数決構造
typedef struct _vote_point_t {
  uint32_t count;
  hough_point_t *point;
} vote_point_t;


#endif
