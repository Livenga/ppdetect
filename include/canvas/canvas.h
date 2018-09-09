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


/* src/canvas/convert.c */
extern canvas_t *
cv_grayscale(const canvas_t *cv_cptr);


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
extern void
filter_free(filter_t *flt_ptr);

#endif
