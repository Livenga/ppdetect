#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>

#include "ppd_type.h"
#include "util.h"
#include "calc/calc.h"
#include "calc/calc.h"
#include "canvas/canvas.h"

//#define _ENABLE_GAUSSIAN 1

bool_t f_color_correction = 0;

static void
print_help(const char *app);
static void
print_version(const char *app);



/* src/calc/harris.c */
extern harris_point_t *
harris_corner_detector(const ncanvas_t *ncv_cptr, double *threshold);


int
main(int argc, char *argv[]) {
  const struct option longopts[] = {
    {"help",             no_argument, 0, 'H'}, // ヘルプ
    {"version",          no_argument, 0, 'V'}, // バージョン
    {"histogram",        no_argument, 0, 'h'},
    {"color-correction", no_argument, 0, 'c'}, // 色補正
    {"div-size",         required_argument, 0, 'd'},
    {0, 0, 0, 0},
  };
  
  int opt, longindex;

  size_t div_size = 5;
  bool_t f_histogram = FALSE;

  
  if(argc < 2) {
    fprintf(stderr, "%s: Operand Error.\n", argv[0]);
    return EOF;
  }

  // ランダムシード
  srand((unsigned)time(NULL));

  char *p_opt;
  while((opt = getopt_long(argc, argv, "HVhcd:",
          longopts, &longindex)) > 0) {
    switch(opt) {
      //
      case 'H':
        print_help(argv[0]);
        break;
      case 'V':
        print_version(argv[0]);
        break;

      case 'h':
        f_histogram = TRUE;
        break;

      case 'd':
        p_opt = (optarg != NULL) ? optarg : argv[optind];
        div_size = strtol(p_opt, NULL, 10);
        break;

      case 'c':
        f_color_correction = TRUE;
        break;
    }
  }


  ncanvas_t *n_ptr;
  canvas_t *target_ptr;

  target_ptr = cv_png_read(argv[argc - 1]);
  n_ptr      = cv2ncv(target_ptr);

  if(f_histogram == TRUE) {
    run_histogram(target_ptr);
  }


  canvas_t  *k_cptr;
  ncanvas_t *k_nptr, *kg_nptr;
  k_nptr = run_kmeans(n_ptr, div_size);
  k_cptr = ncv2cv(k_nptr);

  char buf[1024];
  {
    char *p_str = NULL, *p_end = NULL;
    memset((void *)buf, '\0', sizeof(buf));

    p_str = (char *)argv[argc - 1];
    for(p_str = strchr(p_str, '/');
        p_str != NULL;
        p_str = strchr(p_str, '/')) { p_end = ++p_str; }

    p_str = (p_end != NULL) ? p_end : argv[argc - 1];

    if((p_end = strchr(p_str, '.')) != NULL) {
      const size_t _name_length = (p_end - p_str) + 1;
      char *_name;
      char *_dt = get_datetime_s();

      _name = (char *)calloc(_name_length, sizeof(char));
      memcpy((void *)_name, (const void *)p_str, sizeof(char) * (p_end - p_str));

      if((_name_length + 23) < 1023) {
        sprintf(buf, "%s_%ld_%s.png", _name, div_size, _dt);
      } else {
        sprintf(buf, "output.png");
      }

      // _name 解放
      memset((void *)_name, '\0', sizeof(char) * _name_length);
      free((void *)_name); _name = NULL;
    } else {
      sprintf(buf, "output.png");
    }
  }
  ncv_png_write(buf, k_nptr);

  // 反転
  ncv_inverse(k_nptr);
  kg_nptr = ncv_grayscale(k_nptr);


  // Harris コーナー検出実行
  double threshold;
  harris_point_t *pts_ptr, *pp;

#ifdef _ENABLE_GAUSSIAN
  ncanvas_t *gaussian_nptr;

  filter_t *f_gaussian;
  f_gaussian = filter_create_gaussian(target_ptr->width / 20, target_ptr->width / 20, 1.0);

  gaussian_nptr = filter_convolution(kg_nptr, f_gaussian);
  filter_free(f_gaussian);

  pts_ptr = harris_corner_detector(gaussian_nptr, &threshold);
  
  ncv_free(gaussian_nptr);
#else
  pts_ptr = harris_corner_detector(kg_nptr, &threshold);
#endif

  for(pp = pts_ptr; pp != NULL; pp = pp->next) {
    if(pp->rate >= threshold) {
      //printf("(%d, %d)\n", pp->x, pp->y);
      cv_draw_circuit(target_ptr, pp->x, pp->y, 4, 4);
    }
  }
  cv_png_write("output.png", target_ptr);


  ncv_free(kg_nptr);
  ncv_free(k_nptr);
  cv_free(k_cptr);


  ncv_free(n_ptr);
  cv_free(target_ptr);

  return 0;
}


static void
print_help(const char *app) {
  printf("Usage: %s [OPTIONS] [image path]\n\n", app);
  printf("[Options]\n");
  printf("   --div-size,         -d : 分割サイズ(Default = 5)\n");
  printf("   --color-correction, -c : 色補正(Default = False)\n");
  printf("   --histogram,        -h : ヒストグラム\n");

  exit(0);
}

static void
print_version(const char *app) {
  printf("Version\n");

  exit(0);
}
