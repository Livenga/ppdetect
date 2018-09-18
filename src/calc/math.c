#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

double
calc_euclid(
    const size_t size,
    const double *p1, const double *p2) {
  int i;
  double sum;

  sum = 0.0;
  for(i = 0; i < size; ++i) {
    sum += pow(*(p1 + i) - *(p2 + i), 2.0);
  }
  return sqrt(sum);
}

double
calc_slope(double radian) {
  return -1.0 / tan(radian);
}
