#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>

#include "ppd_type.h"
#include "util.h"
#include "calc/calc.h"
#include "canvas/canvas.h"


bool_t f_color_correction = 0;

static void
print_help(const char *app);
static void
print_version(const char *app);

extern ncanvas_t *
run_kmeans(const ncanvas_t *ncv_cptr, const size_t div_size);


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


  canvas_t *cv_ptr;
  color_t blue = { 0x00, 0x00, 0xFF };
  cv_ptr = cv_alloc(1024, 1024, RGB);
  cv_draw_circuit_c(cv_ptr, 0, 0, 128, 128, blue);

  cv_png_write("output.png", cv_ptr);

  return 0;


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
  ncanvas_t *k_nptr;
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

      sprintf(buf, "%s_%ld_%s.png", _name, div_size, _dt);

      // _name 解放
      memset((void *)_name, '\0', sizeof(char) * _name_length);
      free((void *)_name); _name = NULL;
    } else {
      sprintf(buf, "output.png");
    }
  }
  cv_png_write(buf, k_cptr);

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
