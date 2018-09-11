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
    {"enable-gaussian", no_argument, 0, 'g'},
    {"enable-binarise", no_argument, 0, 'b'},
    {0, 0, 0, 0},
  };
  
  int opt, longindex;

  size_t div_size = 5;
  bool_t f_histogram       = FALSE,
         f_enable_gaussian = FALSE,
         f_enable_binarise = FALSE;

  
  if(argc < 2) {
    fprintf(stderr, "%s: Operand Error.\n", argv[0]);
    return EOF;
  }

  // ランダムシード
  srand((unsigned)time(NULL));

  char *p_opt;
  while((opt = getopt_long(argc, argv, "HVhcd:gb",
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

      case 'g':
        f_enable_gaussian = TRUE;
        break;
      case 'b':
        f_enable_binarise = TRUE;
        break;
    }
  }


  // 画像データの読み込み
  ncanvas_t *n_ptr;
  canvas_t *target_ptr;

  target_ptr = cv_png_read(argv[argc - 1]);
  n_ptr      = cv2ncv(target_ptr);

  // ヒストグラムの生成
  if(f_histogram == TRUE) {
    fprintf(stderr, "* ヒストグラムの生成\n");
    run_histogram(target_ptr);
  }


  //
  char   *output_name_s;
  double bin_threshold;

  canvas_t  *k_cptr;
  ncanvas_t *k_nptr, *kg_nptr;


  fprintf(stderr, "* k平均法の実行\n");
  k_nptr = run_kmeans(n_ptr, div_size, &bin_threshold);
  k_cptr = ncv2cv(k_nptr);

  output_name_s = get_output_filename(argv[argc - 1], "kmeans", "png");
  ncv_png_write(output_name_s, k_nptr);

  // 反転
  //ncv_inverse(k_nptr);
  kg_nptr = ncv_grayscale(k_nptr);


  // Harris コーナー検出実行
  double harris_threshold;
  harris_point_t *pts_ptr, *pp;

  ncanvas_t *ncv_target_ptr;

  // NOTE: コーナー検出を行う前にガウシアンフィルタを適応させることで
  // 検出精度が向上されたと思う.
  //
  // アンチエイリアスをしていないため, 傾斜が存在すると検出精度が低下.
  // (検出点が多くなってしまうことを精度の低下としている)
  if(f_enable_gaussian == TRUE) {
    filter_t *f_gaussian;
    
    //
    if(f_enable_binarise == TRUE) {
      fprintf(stderr, "* 2値化処理の実行(閾値: %f)\n", bin_threshold);
      ncv_binarize(kg_nptr, bin_threshold);
    }

    // XXX: ガウシアンフィルタのサイズとシグマの値は,
    // 現在結果を見て判断している.
    // 画像サイズとの関係性が分かり次第修正する必要がある.
    //f_gaussian = filter_create_gaussian(8, 8, bin_threshold);
    f_gaussian = filter_create_gaussian(8, 8, 0.65);
    fprintf(stderr, "* ガウシアンフィルタを適応(%dx%d)\n", f_gaussian->width, f_gaussian->height);

    ncv_target_ptr = filter_convolution(kg_nptr, f_gaussian);
    filter_free(f_gaussian);


    // 画像出力
    output_name_s = get_output_filename(argv[argc - 1], "gaussian", "png");
    ncv_png_write(output_name_s, ncv_target_ptr);
  } else {
    if(f_enable_binarise == TRUE) {
      fprintf(stderr, "* 2値化処理の実行(閾値: %f)\n", bin_threshold);
      ncv_binarize(kg_nptr, bin_threshold);
    }

    ncv_target_ptr = kg_nptr;
  }

  // Harris コーナー検出の実行
  fprintf(stderr, "* Harris コーナー検出実行\n");
  pts_ptr = harris_corner_detector(ncv_target_ptr, &harris_threshold);


  for(pp = pts_ptr; pp != NULL; pp = pp->next) {
    if(pp->rate >= harris_threshold) {
      //printf("(%d, %d)\n", pp->x, pp->y);
      cv_draw_circuit(target_ptr, pp->x, pp->y, 24, 24);
    }
  }

  output_name_s = get_output_filename(argv[argc - 1], "harris", "png");
  cv_png_write(output_name_s, target_ptr);


  //ncv_free(kg_nptr);
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
