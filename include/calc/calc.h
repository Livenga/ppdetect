#ifndef _CALC_H
#define _CALC_H

#include "canvas/canvas.h"
#include "calc/calc_type.h"

/* src/calc/histogram.c */
extern void
run_histogram(const canvas_t *cv_cptr);


/* src/calc/kmeans.c */
extern ncanvas_t *
run_kmeans(const ncanvas_t *ncv_cptr, const size_t div_size, double *out_threshold);

/* src/calc/harris.c */
extern harris_point_t *
harris_point_new(uint32_t x, uint32_t y, double rate);
extern size_t
harris_point_count(const harris_point_t *p);
extern void
harris_point_release(harris_point_t *pt_ptr);

extern harris_point_t *
harris_corner_detector(const ncanvas_t *ncv_cptr);


/* src/calc/hough.c */
extern hough_point_t *
hough_point_new(uint32_t x, uint32_t y,
    double radian, double rho);

extern size_t
hough_point_count(const hough_point_t *hp);

extern void
hough_point_release(hough_point_t *p);
extern void
hough_points_release(hough_point_t **hp_ptr);

extern hough_point_t *
run_hough_transform(
    const harris_point_t *hr_points);
#if 0
extern hough_point_t **
run_hough_transform(
    const harris_point_t *harris_points,
    const double          harris_threshold);
#endif

#endif
