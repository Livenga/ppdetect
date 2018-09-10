#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <png.h>

#include "canvas/canvas.h"


static int
_png_color_type(int data_size) {
  switch(data_size) {
    case 1: return PNG_COLOR_TYPE_GRAY;
    case 2: return PNG_COLOR_TYPE_GA;
    case 3: return PNG_COLOR_TYPE_RGB;
    case 4: return PNG_COLOR_TYPE_RGBA;
  }

  return -1;
}


/**
 * \brief PNGから読み込み
 *
 * \retval [
 * canvas_t *:
 * NULL: 異常終了
 * ]
 */
canvas_t *
cv_png_read(const char *png_path) {
  FILE *png_fp;

  png_structp png_ptr;
  png_infop   info_ptr;
  png_byte    png_color;
  uchar_t     **png_data;

  int      i;
  uint32_t width, height, data_size;
  color_type_t  color_type;
  canvas_t *png_cv;


  if(png_path == NULL || (png_fp = fopen(png_path, "rb")) == NULL) {
    return NULL;
  }

  png_ptr  = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  info_ptr = png_create_info_struct(png_ptr);

  png_init_io(png_ptr, png_fp);

  png_read_info(png_ptr, info_ptr);

  width      = png_get_image_width(png_ptr, info_ptr);
  height     = png_get_image_height(png_ptr, info_ptr);
  png_color  = png_get_color_type(png_ptr, info_ptr);

  switch(png_color) {
    case PNG_COLOR_TYPE_GRAY:
      color_type = GRAY;
      break;

    case PNG_COLOR_TYPE_GA:
      color_type = GA;
      break;

    case PNG_COLOR_TYPE_RGB:
      color_type = RGB;
      break;

    case PNG_COLOR_TYPE_RGBA:
      color_type = RGBA;
      break;

    default:
      png_cv = NULL;
      goto pt_return;
  }

  data_size = cv_get_data_size_v(color_type);
  png_data  = (uchar_t **)calloc(height, sizeof(uchar_t *));
  for(i = 0; i < height; ++i) {
    png_data[i] = (uchar_t *)calloc(width * data_size, sizeof(uchar_t));
  }

  png_read_image(png_ptr, png_data);
  png_read_end(png_ptr, info_ptr);

  png_destroy_info_struct(png_ptr, &info_ptr);
  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

  fclose(png_fp);


  const size_t line_size = sizeof(uchar_t) * width * cv_get_data_size_v(color_type);

  png_cv = cv_alloc(width, height, color_type);
  if(png_cv == NULL) {
    goto pt_data_release;
  }

  for(i = 0; i < height; ++i) {
    memcpy((void *)(png_cv->data + (i * line_size)), (const void *)png_data[i], line_size);
  }

pt_data_release:
  for(i = 0; i < height; ++i) {
    memset((void *)png_data[i], '\0', line_size);
    free((void *)png_data[i]);
    png_data[i] = NULL;
  }
  free((void *)png_data);
  png_data = NULL;

pt_return:
  return png_cv;
}


/**
 * \brief PNGを[0.0 - 1.0] で正規化したデータで読み込む
 *
 * [
 * ncanvas_t *:
 * NULL: 異常終了
 * ]
 */
ncanvas_t *
cv_png_read_n(const char *png_path) {
  canvas_t  *cv_ptr;
  ncanvas_t *ncv_ptr;


  if((cv_ptr = cv_png_read(png_path)) == NULL) {
    return NULL;
  }
  ncv_ptr = cv2ncv((const canvas_t *)cv_ptr);

  cv_free(cv_ptr);
  return ncv_ptr;
}


/**
 */
int
cv_png_write(const char *png_path, const canvas_t *cv_cptr) {
  FILE        *png_fp;
  png_structp png_ptr;
  png_infop   info_ptr;
  int         png_color_type;
  uchar_t     **png_data;


  if(cv_cptr == NULL) { return EOF; }
  if((png_color_type = _png_color_type(cv_cptr->color_type)) == -1) { return EOF; }

  if((png_fp = fopen(png_path, "wb")) == NULL) {
    return EOF;
  }

  png_ptr  = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  info_ptr = png_create_info_struct(png_ptr);


  png_init_io(png_ptr, png_fp);

  png_set_IHDR(png_ptr, info_ptr,
      cv_cptr->width, cv_cptr->height, 8, png_color_type,
      PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);


  int i;
  const size_t line_size = sizeof(uchar_t) * cv_cptr->width * cv_get_data_size_v(cv_cptr->color_type);

  png_data = (uchar_t **)calloc(cv_cptr->height, sizeof(uchar_t *));
  for(i = 0; i < cv_cptr->height; ++i) {
    png_data[i] = (uchar_t *)calloc(line_size, 1);
    memcpy((void *)png_data[i], (const void *)(cv_cptr->data + (i * line_size)), line_size);
  }

  png_write_info(png_ptr, info_ptr);
  png_write_image(png_ptr, png_data);
  png_write_end(png_ptr, info_ptr);


  png_destroy_info_struct(png_ptr, &info_ptr);
  png_destroy_write_struct(&png_ptr, &info_ptr);

  fclose(png_fp);


  for(i = 0; i < cv_cptr->height; ++i) {
    memset((void *)png_data[i], '\0', line_size);

    free((void *)png_data[i]);
    png_data[i] = NULL;
  }
  free((void *)png_data);
  png_data = NULL;


  return 0;
}


int
ncv_png_write(const char *path, const ncanvas_t *ncv_cptr) {
  int status;
  canvas_t *cv_ptr;

  status = EOF;
  if((cv_ptr = ncv2cv(ncv_cptr)) != NULL) {
    status = cv_png_write(path, cv_ptr);
    cv_free(cv_ptr);
  }

  return status;
}
