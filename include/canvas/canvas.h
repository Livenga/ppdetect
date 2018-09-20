#ifndef _CANVAS_H
#define _CANVAS_H

#ifndef _CANVAS_DEF_H
#include "canvas_def.h"
#endif

#ifndef _CANVAS_TYPE_H
#include "canvas_type.h"
#endif


/* src/canvas/alloc.c */
extern uint8_t
cv_get_data_size(canvas_common_t *cv_ptr);
extern uint8_t
cv_get_data_size_v(color_type_t color_type);

extern canvas_t *
cv_alloc(uint32_t width, uint32_t height, color_type_t color_type);
extern ncanvas_t *
ncv_alloc(uint32_t width, uint32_t height, color_type_t color_type);

extern void
cv_free(canvas_t *cv_ptr);
extern void
ncv_free(ncanvas_t *ncv_ptr);


/* src/canvas/png.c */
extern canvas_t *
cv_png_read(const char *png_path);
extern ncanvas_t *
cv_png_read_n(const char *png_path);
extern int
cv_png_write(const char *png_path, const canvas_t *cv_cptr);
extern int
ncv_png_write(const char *path, const ncanvas_t *ncv_cptr);


/* src/canvas/convert.c */
extern canvas_t *
cv_grayscale(const canvas_t *cv_cptr);
extern ncanvas_t *
ncv_grayscale(const ncanvas_t *ncv_cptr);

extern void
ncv_inverse(ncanvas_t *ncv_ptr);

extern void
ncv_binarize(ncanvas_t *ncv_ptr, double threshold);
// 線形補間
extern canvas_t *
cv_bilinear(const canvas_t *cv_cptr, uint32_t width, uint32_t height);
extern canvas_t *
cv_bilinear_magnif(const canvas_t *cv_cptr, const double magnif);



/* src/canvas/util.c */
extern ncanvas_t *
cv2ncv(const canvas_t *cv_cptr);
extern canvas_t *
ncv2cv(const ncanvas_t *ncv_cptr);

extern canvas_t *
cv_draw_circuit_c(canvas_t *self,
    int x,     int y,
    int width, int height,
    color_t color);

extern canvas_t *
cv_draw_circuit(canvas_t *self,
    int x,     int y,
    int width, int height);


/* src/canvas/filter.c */
extern filter_t *
filter_create_gaussian(uint32_t width, uint32_t height, double sigma);

extern filter_t *
filter_create_sobel_x(void);
extern filter_t *
filter_create_sobel_y(void);

extern double
filter_convolution_partial(
    const ncanvas_t *self,
    uint32_t        pos_x,
    uint32_t        pos_y,
    const filter_t  *filter,
    double          outside);

extern ncanvas_t *
filter_convolution(ncanvas_t *ncv_ptr, const filter_t *filter);

extern void
filter_free(filter_t *flt_ptr);

#endif
