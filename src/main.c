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
#include "canvas/canvas.h"

#include "temp.h"

#define DEG2RAD(deg) (((deg) * M_PI) / 180.0)
#define RAD2DEG(rad) (((rad) * 180.0) / M_PI)


bool_t f_color_correction = 0;

static void
print_help(const char *app);
static void
print_version(const char *app);

void
hough_draw_line_c(canvas_t *self, double rho, double radian, color_t color) {
  int x, y;

  for(x = 0; x < self->width; ++x) {
    double _a = -x  / tan(radian),
           _c = (radian != 0) ? rho / sin(radian) : 0.0;

    y = _a + _c;

    if(y >= 0 && y < (self->height - 1)) {
      *(self->data + ((y * self->width + x) * self->color_type + 0)) = color.r;
      *(self->data + ((y * self->width + x) * self->color_type + 1)) = color.g;
      *(self->data + ((y * self->width + x) * self->color_type + 2)) = color.b;
    }
  }
}

void
hough_draw_line(canvas_t *self, double rho, double radian) {
  const color_t blue = {0x00, 0x00, 0xFF}, red = {0xFF, 0x00, 0x00};

  hough_draw_line_c(self, rho, radian, blue);
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

  int i, j;
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
  // 2018/09/16
  // 斜辺上の検出点が多くなれば, 直線に近い状態である場合に限るが
  // Hough変換を行う際に有利になる.
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
  size_t nof_harris_points;
  harris_point_t *harris_points;


  fprintf(stderr, "* Harris コーナー検出実行\n");
  harris_points     = harris_corner_detector(ncv_target_ptr);
  nof_harris_points = harris_point_count(harris_points);

  fprintf(stderr, "* Harris コーナー検出個数: %ld\n", nof_harris_points);

#if 1
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
  //cv_png_write(output_name_s, target_ptr);
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


    const uint32_t _rho_size  = (uint32_t)_max_rho + 1;
    const uint32_t _vote_size = _rho_size * 181;
    const double   _min_rad   = DEG2RAD(-90.0), _max_rad  = DEG2RAD(90.0),
          _range_rad = _max_rad - _min_rad;


    // 多数決
    vote_point_t *vote_pp;
    vote_pp = (vote_point_t *)calloc(_vote_size, sizeof(vote_point_t));

    // 整数型として, rho, radian を二次元空間で投票を
    // 行うため正規化をする.
    for(_ho_cur = hough_points; _ho_cur != NULL; _ho_cur = _ho_cur->next) {
      uint32_t offset, i_rho, i_theta;
      vote_point_t *_vpp;

      //_ho_cur->rho    = (_ho_cur->rho    - _min_rho) / _range_rho;
      //_ho_cur->radian = (_ho_cur->radian - _min_rad) / _range_rad;

      i_rho   = (int32_t)(_rho_size * ((_ho_cur->rho - _min_rho) / _range_rho));
      i_theta = (int32_t)(RAD2DEG(((_ho_cur->radian - _min_rad) / _range_rad)
            * _range_rad + _min_rad) + 90);
      offset  = i_rho * 181 + i_theta;

      _vpp         = (vote_pp + offset);
      _vpp->point  = _ho_cur;
      ++_vpp->count;
    }


#if 0
    // 多数決で投票したデータを(x, y, Count)の三次元データとしてCSVファイルに出力
    FILE *vote_csv_fp;

    output_name_s = get_output_filename(argv[argc - 1], "vote", "csv");
    if((vote_csv_fp = fopen(output_name_s, "w")) != NULL) {
      for(i = 0; i < _rho_size; ++i) {
        for(j = 0; j < 181; ++j) {
          fprintf(vote_csv_fp, "%d %d %u\n", j, i, (vote_pp + (i * 181 + j))->count);
        }
        fprintf(vote_csv_fp, "\n");
      }

      fclose(vote_csv_fp);
    }
#endif

    //memset((void *)scores, '\0', sizeof(score_t) * ((_rho_size * 181) / 100));
    //free((void *)scores); scores = NULL;


#if 1
    // カウントソート
    int (*_vote_comp)(const void *, const void *) = ({
        int __comparator(const void *p1, const void *p2) {
        return ((vote_point_t *)p2)->count > ((vote_point_t *)p1)->count;
        }
        __comparator;
        });
    qsort((void *)vote_pp, _vote_size, sizeof(vote_point_t), _vote_comp);

    // 基準パラメータ
#define TOP_N 64
    const vote_point_t *base_vpp = (vote_pp + 0);
    const double base_offset = base_vpp->point->rho / sin(base_vpp->point->radian);
    const double base_slope  = calc_slope(base_vpp->point->radian);

    // 対になる基準線
    vote_point_t *pol_vpp = NULL;
    double emax_pol;


    fprintf(stderr, "* 基準 rho: %f, radian: %f, slope: %f(%f)\n",
        base_vpp->point->rho,
        base_vpp->point->radian,
        base_slope, RAD2DEG(base_slope));
    hough_draw_line(target_ptr, base_vpp->point->rho, base_vpp->point->radian);

#define X_OFFSET(rho, radian) (rho / cos(radian))
#define Y_OFFSET(rho, radian) (rho / sin(radian))

    const bool_t is_x = fabs((RAD2DEG(base_vpp->point->radian))) >= 45.0 ? FALSE : TRUE;
    emax_pol = 0.0;
    for(i = 1; i < TOP_N; ++i) {
      vote_point_t *_vpp = (vote_pp + i);
      double _slope, _e_slope, _offset, _e_offset;

      if(_vpp->point == NULL) { continue; }

      _slope   = calc_slope(_vpp->point->radian);
      _e_slope = calc_euclid(1, &base_slope, &_slope);

      if(_e_slope < 0.60) {
        fprintf(stderr, "* diff: %f, %f\n",
            _e_slope, _vpp->point->rho / sin(_vpp->point->radian));

          _offset = (is_x == TRUE)
            ? X_OFFSET(_vpp->point->rho, _vpp->point->radian)
            : Y_OFFSET(_vpp->point->rho, _vpp->point->radian);

        _e_offset = calc_euclid(1, &base_offset, &_offset);

        if(_e_offset > emax_pol) {
          emax_pol = _e_offset;
          pol_vpp = _vpp;
        }
      }
    }

    const vote_point_t *vertical_vpp;
    if(pol_vpp != NULL) {
      fprintf(stderr, "* 対基準 rho: %f, radian: %f\n",
          pol_vpp->point->rho, pol_vpp->point->radian);

      hough_draw_line(target_ptr, pol_vpp->point->rho, pol_vpp->point->radian);
      vertical_vpp = (fabs(base_vpp->point->radian) > fabs(pol_vpp->point->radian))
        ? pol_vpp : base_vpp;
    } else {
      vertical_vpp = base_vpp;
    }

    const double rad90 = M_PI / 2.0;
    for(i = 1; i < _vote_size; ++i) {
      double _rad;
      vote_point_t *_vpp = (vote_pp + i);

      if(_vpp == NULL || _vpp->point == NULL
          || _vpp == base_vpp || _vpp == vertical_vpp) { continue; }

      _rad = vertical_vpp->point->radian - _vpp->point->radian;
      _rad = calc_euclid(1, &rad90, &_rad);
    }

    // 基準パラメータと上位の直線パラメータを比較
    fprintf(stderr, "* 検出した上位の直線パラメータ\n");
    for(i = 0; i < TOP_N; ++i) {}


    output_name_s = get_output_filename(argv[argc - 1], "draw_line", "png");
    cv_png_write(output_name_s, target_ptr);
#endif


    memset ((void *)vote_pp, '\0', sizeof(vote_point_t) * _vote_size);
    free((void *)vote_pp);
    vote_pp = NULL;
    // rho, theta
#if 0 // dsc0023_5.png
    hough_draw_line(target_ptr, 71.513, -0.9091);
    hough_draw_line(target_ptr, 80.3, -0.924);
    hough_draw_line(target_ptr, 94.42, 0.818);
    hough_draw_line(target_ptr, 230.305, 0.8);
    hough_draw_line(target_ptr, 112.12, -1.03641);
    hough_draw_line(target_ptr, 0.0, -0.7455);
    output_name_s = get_output_filename(argv[argc - 1], "hough", "png");
    cv_png_write(output_name_s, target_ptr);
#endif

    hough_point_release(hough_points);
  }

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
