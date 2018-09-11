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

#endif
