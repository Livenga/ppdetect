#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "ppd_type.h"
#include "calc/calc_type.h"
#include "canvas/canvas.h"
#include "util.h"


void
run_hough_transform(
    const harris_point_t *harris_points,
    const double          harris_threshold) {
  const harris_point_t  *harris_cursor;

  char *csv_name;
  FILE *csv_fp;

  double rho, theta;


  csv_name = get_output_filename("hough", NULL, "csv");
  csv_fp   = fopen(csv_name, "w");

  for(harris_cursor  = harris_points;
      harris_cursor != NULL;
      harris_cursor  = harris_cursor->next) {
    if(harris_cursor->rate >= harris_threshold) {
      for(theta = -90.0; theta <= 90.0; theta += 0.1) {
        const double _rad = (theta * M_PI) / 180.0;

        rho = harris_cursor->x * cos(_rad)
          + harris_cursor->y * sin(_rad);

        if(csv_fp != NULL) { fprintf(csv_fp, "%f %f\n", theta, rho); }
      }

      //if(csv_fp != NULL) { fprintf(csv_fp, "\n"); }
    }
  }


  if(csv_fp != NULL) { fclose(csv_fp); }
}
