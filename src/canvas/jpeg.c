#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "canvas/canvas.h"
#undef TRUE
#undef FALSE
#include <jpeglib.h>


canvas_t *
cv_jpeg_read(const char *path) {
  FILE *jpeg_fp;
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;

  canvas_t *ret;


  if((jpeg_fp = fopen(path, "rb")) == NULL) {
    return NULL;
  }

  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&cinfo);

  jpeg_stdio_src(&cinfo, jpeg_fp);


  int i;
  uchar_t **lines;


  jpeg_read_header(&cinfo, TRUE);
  jpeg_start_decompress(&cinfo);

  lines = (uchar_t **)calloc(cinfo.output_height, sizeof(uchar_t *));
  for(i = 0; i < cinfo.output_height; ++i) {
    *(lines + i) = (uchar_t *)calloc(cinfo.output_width * cinfo.out_color_components,
        sizeof(uchar_t));
  }

  while(cinfo.output_scanline < cinfo.output_height) {
    jpeg_read_scanlines(&cinfo,
        lines + cinfo.output_scanline,
        cinfo.output_height - cinfo.output_scanline);
  }

  jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);

  fclose(jpeg_fp);


  // canvas_t * に変換
  ret = cv_alloc(cinfo.output_width, cinfo.output_height, cinfo.out_color_components);
  const size_t line_size = cinfo.output_width * cinfo.out_color_components;

  for(i = 0; i < cinfo.output_height; ++i) {
    const size_t _offset = i * line_size;
    memcpy((void *)(ret->data + _offset), (const void *)*(lines + i), line_size);
    memset((void *)*(lines + i), '\0', sizeof(char) * line_size);

    free((void *)*(lines + i));
    *(lines + i) = NULL;
  }
  free((void *)lines); lines = NULL;


  return ret;
}
