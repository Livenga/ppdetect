#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "canvas/canvas.h"
#include "util.h"


static void
_calc_rgb_histogram(const canvas_t *cv_cptr, uint32_t *r, uint32_t *g, uint32_t *b) {
  int i, j;

  for(i = 0; i < cv_cptr->height; ++i) {
    for(j = 0; j < cv_cptr->width; ++j) {
      const size_t _offset = (i * cv_cptr->width + j) * cv_cptr->color_type;

      ++*(r + cv_cptr->data[_offset]);
      ++*(g + cv_cptr->data[_offset + 1]);
      ++*(b + cv_cptr->data[_offset + 2]);
    }
  }
}

static void
_calc_g_histogram(const canvas_t *cv_cptr, uint32_t *g) {
  int i, j;

  for(i = 0; i < cv_cptr->height; ++i) {
    for(j = 0; j < cv_cptr->width; ++j) {
      const size_t _offset = (i * cv_cptr->width + j) * cv_cptr->color_type;

      ++*(g + cv_cptr->data[_offset]);
    }
  }
}

static void
_save_histogram(const char *path, const uint32_t *arr) {
  FILE *csv_fp;
  int i;


  if((csv_fp = fopen(path, "w")) != NULL) {
    for(i = 0; i < 256; ++i) {
      fprintf(csv_fp, "%d %u\n", i, *(arr + i));
    }
    fclose(csv_fp);
  }
}


void
run_histogram(const canvas_t *cv_cptr) {
  const char *str_dt = get_datetime_s();
  char *csv_path = NULL;


  if(cv_cptr == NULL || cv_cptr->data == NULL) {
    fprintf(stderr, "* 対象データが読み込まれていません.");
    return;
  }


  if(is_exists("./csv") == TRUE && is_directory("./csv") == FALSE) {
    return;
  } else if(is_exists("./csv") == FALSE) {
    if(mkdir("./csv", 0755) != 0) {
      eprintf(stderr, "mkdir(2)", "./csv");
      return;
    }
  }

  csv_path = (char *)calloc(1024, sizeof(char));

  if(cv_cptr->color_type == RGB || cv_cptr->color_type == RGBA) {
    uint32_t *r_arr, *g_arr, *b_arr;

    r_arr = (uint32_t *)calloc(256, sizeof(uint32_t));
    g_arr = (uint32_t *)calloc(256, sizeof(uint32_t));
    b_arr = (uint32_t *)calloc(256, sizeof(uint32_t));

    _calc_rgb_histogram(cv_cptr, r_arr, g_arr, b_arr);

    // 保存
    sprintf(csv_path, "./csv/r_%s", str_dt);
    _save_histogram(csv_path, (const uint32_t *)r_arr);

    sprintf(csv_path, "./csv/g_%s", str_dt);
    _save_histogram(csv_path, (const uint32_t *)g_arr);
    
    sprintf(csv_path, "./csv/b_%s", str_dt);
    _save_histogram(csv_path, (const uint32_t *)b_arr);

    memset((void *)r_arr, '\0', sizeof(uint32_t) * 256);
    memset((void *)g_arr, '\0', sizeof(uint32_t) * 256);
    memset((void *)b_arr, '\0', sizeof(uint32_t) * 256);

    free((void *)r_arr); r_arr = NULL;
    free((void *)g_arr); g_arr = NULL;
    free((void *)b_arr); b_arr = NULL;
  } else if(cv_cptr->color_type == GRAY || cv_cptr->color_type == GA) {
    uint32_t *g_arr;

    g_arr = (uint32_t *)calloc(256, sizeof(uint32_t));

    // 計算
    _calc_g_histogram(cv_cptr, g_arr);
    // 保存
    sprintf(csv_path, "./csv/g_%s", str_dt);
    _save_histogram(csv_path, (uint32_t *)g_arr);

    memset((void *)g_arr, '\0', sizeof(uint32_t) * 256);
    free((void *)g_arr); g_arr = NULL;
  }

  memset((void *)csv_path, '\0', sizeof(char) * 1024);
  free((void *)csv_path);
  csv_path = NULL;
}
