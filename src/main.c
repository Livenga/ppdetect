#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>
#include <math.h>

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

void
hough_draw_line(canvas_t *self, double rho, double radian) {
  int x, y;

  for(x = 0; x < self->width; ++x) {
    y = (int)((rho - x * cos(radian)) / sin(radian));

    if(y >= 0 && y < (self->height - 1)) {
      *(self->data + ((y * self->width + x) * self->color_type + 0)) = 0x00;
      *(self->data + ((y * self->width + x) * self->color_type + 1)) = 0x00;
      *(self->data + ((y * self->width + x) * self->color_type + 2)) = 0xFF;
    }
  }
}

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
  //ncv_png_write(output_name_s, k_nptr);

  // 反転
  //ncv_inverse(k_nptr);
  kg_nptr = ncv_grayscale(k_nptr);


  // 前処理
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
    //output_name_s = get_output_filename(argv[argc - 1], "gaussian", "png");
    //ncv_png_write(output_name_s, ncv_target_ptr);
  } else {
    if(f_enable_binarise == TRUE) {
      fprintf(stderr, "* 2値化処理の実行(閾値: %f)\n", bin_threshold);
      ncv_binarize(kg_nptr, bin_threshold);
    }

    ncv_target_ptr = kg_nptr;
  }
  // 画像出力
  output_name_s = get_output_filename(argv[argc - 1], "pre_processed", "png");
  ncv_png_write(output_name_s, ncv_target_ptr);


  // Harris コーナー検出の実行
  harris_point_t *harris_points;


  fprintf(stderr, "* Harris コーナー検出実行\n");
  harris_points = harris_corner_detector(ncv_target_ptr);
  fprintf(stderr, "* Harris コーナー検出個数: %ld\n", harris_point_count(harris_points));

#if 0
  harris_point_t *harris_cursor;

  // 元画像に対してい, 検出した点を中心にした円を描く
  fprintf(stderr, "* 元画像(%s)に円を描く.\n", argv[argc - 1]);
  for(harris_cursor  = harris_points;
      harris_cursor != NULL;
      harris_cursor  = harris_cursor->next) {
    cv_draw_circuit(target_ptr,
        harris_cursor->x, harris_cursor->y,
        8, 8);
  }
  // 保存
  output_name_s = get_output_filename(argv[argc - 1], "harris", "png");
  cv_png_write(output_name_s, target_ptr);
#endif



  // Hough 変換
  hough_point_t *hough_points;

  hough_points = run_hough_transform(harris_points);
  if(hough_points != NULL) {
    double _min_rho, _max_rho, _range_rho;
    hough_point_t *_ho_cur;


    fprintf(stderr, "* Hough 変換: %ld\n", hough_point_count(hough_points));

    // 最小値, 最大値の取得
    _min_rho = hough_points->rho;
    _max_rho = hough_points->rho;
    for(_ho_cur = hough_points->next; _ho_cur != NULL; _ho_cur = _ho_cur->next) {
      if(_min_rho > _ho_cur->rho) {
        _min_rho = _ho_cur->rho;
      } else if(_max_rho < _ho_cur->rho) {
        _max_rho = _ho_cur->rho;
      }
    }
    // 範囲(Range)
    _range_rho = _max_rho - _min_rho;

#define DEG2RAD(deg) (((deg) * M_PI) / 180.0)
#define RAD2DEG(rad) (((rad) * 180.0) / M_PI)

    const double _min_rad = DEG2RAD(-90.0),
          _max_rad   = DEG2RAD(90.0),
          _range_rad = _max_rad - _min_rad;


    uint32_t *vote;
    vote = (uint32_t *)calloc(1025 * 181, sizeof(uint32_t));

   // 正規化
   for(_ho_cur = hough_points; _ho_cur != NULL; _ho_cur = _ho_cur->next) {
     int32_t i_rho, i_theta;
     uint32_t offset;

     _ho_cur->rho    = (_ho_cur->rho    - _min_rho) / _range_rho;
     _ho_cur->radian = (_ho_cur->radian - _min_rad) / _range_rad;

     i_rho   = (int32_t)(1024 * _ho_cur->rho);
     i_theta = (int32_t)(RAD2DEG(_ho_cur->radian * _range_rad + _min_rad) + 90);

     offset = i_rho * 180 + i_theta;
     ++(*(vote + offset));
   }

   // rho, theta
#if 0 // dsc0023_5.png
   hough_draw_line(target_ptr, 71.513, -0.9091);
   hough_draw_line(target_ptr, 80.3, -0.924);
   hough_draw_line(target_ptr, 94.42, 0.818);
   hough_draw_line(target_ptr, 230.305, 0.8);
   hough_draw_line(target_ptr, 112.12, -1.03641);
   hough_draw_line(target_ptr, 0.0, -0.7455);
#endif


#if 0
   int i, j;
   for(i = 0; i <= 1024; ++i) {
     for(j = 0; j <= 180; ++j) {
       uint32_t v = *(vote + (i * 181 + j));

       if(v > 5) {
         double rad = DEG2RAD((j - 90));
         double rho = (((double)i / 1024.0) * _range_rho) + _min_rho;

         hough_draw_line(target_ptr, rho, rad);
       }
     }
   }

#endif
  output_name_s = get_output_filename(argv[argc - 1], "hough", "png");
  cv_png_write(output_name_s, target_ptr);

  hough_point_release(hough_points);
  }
#if 0
  // Hough変換
  int32_t i;
  double min_rho, max_rho, range_rho;
  hough_point_t **hp_ptr, *hp_cur;

  fprintf(stderr, "* [T] Hough 変換\n");
  hp_ptr = run_hough_transform(harris_points, harris_threshold);


  min_rho = (*hp_ptr)->rho;
  max_rho = (*hp_ptr)->rho;
  for(i = 0; i < NUMBER_OF_HOUGH_POINT; ++i) {
    hough_point_t *_hp = *(hp_ptr + i);

    for(hp_cur = _hp; hp_cur != NULL; hp_cur = hp_cur->next) {
      if(min_rho > hp_cur->rho) {
        min_rho = hp_cur->rho;
      } else if(max_rho < hp_cur->rho) {
        max_rho = hp_cur->rho;
      }
    }
  }
  range_rho = max_rho - min_rho;

  hough_points_release(hp_ptr);
#endif

  harris_point_release(harris_points);

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
  printf("   --enable-binarise,  -b : 2値化\n");
  printf("   --enable-gaussian,  -g : Gaussian フィルタの適応\n");

  exit(0);
}

static void
print_version(const char *app) {
  printf("Version\n");

  exit(0);
}
