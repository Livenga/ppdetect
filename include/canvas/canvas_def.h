#ifndef _CANVAS_DEF_H
#define _CANVAS_DEF_H

#define CANVAS_SIZE(cv) \
  (cv.width * cv.height * cv_get_data_size_v(cv.color_type))

#define PCANVAS_SIZE(cv_ptr) \
  (cv_ptr->width * cv_ptr->height * cv_get_data_size_v(cv_ptr->color_type))

#endif
