#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "ppd_type.h"

#include "canvas/canvas.h"
#include "calc/calc_type.h"
#include "calc/k-means.h"


#if 0
static double
_calc_euclid(cluster_t *c_ptr, gcenter_t *g_ptr) {
  double v0, v1, v2;

  v0 = c_ptr->value[0] - g_ptr->value[0];
  v1 = c_ptr->value[1] - g_ptr->value[1];
  v2 = c_ptr->value[2] - g_ptr->value[2];

  return sqrt(v0 * v0 + v1 * v1 + v2 * v2);
}
#else
static double
_calc_euclid(const double *ptr0, const double *ptr1, size_t size) {
  int i;
  double sum;

  sum = 0.0;
  for(i = 0; i < size; ++i) {
    double v;

    v = *(ptr0 + i) - *(ptr1 + i);
    sum += (v * v);
  }

  return sqrt(sum);
}
#endif


ncanvas_t *
run_kmeans(const ncanvas_t *ncv_cptr, const size_t div_size) {
  int i, j;
  double    *euclids;

  cluster_t *clusters, *_c_ptr;
  gcenter_t *gcenters, *pre_gcenters, *_g_ptr;


  if(ncv_cptr == NULL) { return NULL; }


  const uint32_t nof_cluster = ncv_cptr->width * ncv_cptr->height;

  clusters     = (cluster_t *)calloc(nof_cluster, sizeof(cluster_t));
  gcenters     = (gcenter_t *)calloc(div_size, sizeof(gcenter_t));
  pre_gcenters = (gcenter_t *)calloc(div_size, sizeof(gcenter_t));

  euclids  = (double *)calloc(div_size, sizeof(double));


  for(i = 0; i < nof_cluster; ++i) {
    const size_t _offset = i * ncv_cptr->color_type;

    _c_ptr = (clusters + i);

    // ランダムにクラスタIDの割当
    _c_ptr->id = rand() % div_size;

    _c_ptr->value[0] = *(ncv_cptr->data + (_offset + 0));
    _c_ptr->value[1] = *(ncv_cptr->data + (_offset + 1));
    _c_ptr->value[2] = *(ncv_cptr->data + (_offset + 2));
  }

  while(1) {
    // クラスタの中央値を初期化
    memset((void *)gcenters, '\0', sizeof(gcenter_t) * div_size);

    // クラスタIDに該当する要素の値の総和と個数を取得
    for(i = 0; i < nof_cluster; ++i) {
      _c_ptr = (clusters + i);
      _g_ptr = (gcenters + _c_ptr->id);

      ++_g_ptr->count;

      _g_ptr->value[0] += _c_ptr->value[0];
      _g_ptr->value[1] += _c_ptr->value[1];
      _g_ptr->value[2] += _c_ptr->value[2];
    }

    // クラスタの中心値を計算
    for(i = 0; i < div_size; ++i) {
      _g_ptr = (gcenters + i);

      if(_g_ptr->count != 0) {
        _g_ptr->value[0] /= _g_ptr->count;
        _g_ptr->value[1] /= _g_ptr->count;
        _g_ptr->value[2] /= _g_ptr->count;
      } else {
        _g_ptr->value[0] = -100.0;
        _g_ptr->value[1] = -100.0;
        _g_ptr->value[2] = -100.0;
      }

#if 0
      printf("%3d: %u(%f, %f, %f)\n",
          i, (gcenters + i)->count,
          _g_ptr->value[0], _g_ptr->value[1], _g_ptr->value[2]
          );
#endif
    }

    //
    for(i = 0; i < nof_cluster; ++i) {
      int    _min_id;
      double _min_euclid;

      _c_ptr = (clusters + i);

      // ユークリッド距離
      for(j = 0; j < div_size; ++j) {
        *(euclids + j) = _calc_euclid(_c_ptr->value, (gcenters + j)->value, 3);
      }

      // 最も近い
      _min_id     = 0;
      _min_euclid = *(euclids);
      for(j = 1; j < div_size; ++j) {
        if(_min_euclid > *(euclids + j)) {
          _min_id     = j;
          _min_euclid = *(euclids + j);
        }
      }

      _c_ptr->id = _min_id;
    }

    // TODO: 終了条件はクラスタの中央値に変化がなくなった場合.
    bool_t _f_break = TRUE;

    for(i = 0; i < div_size; ++i) {
      if(_f_break == FALSE) { break; }

      for(j = 0; j < 3; ++j) {
        if((gcenters + i)->value[j] != (pre_gcenters + i)->value[j]) {
          _f_break = FALSE;
          break;
        }
      }
    }

    if(_f_break) { break; }
    memcpy((void *)pre_gcenters, (const void *)gcenters, sizeof(gcenter_t) * div_size);

    //if(++count == 30) { break; }

  }

  // 
  printf("* クラスタの中央値\n");
  for(i = 0; i < div_size; ++i) {
    _g_ptr = (gcenters + i);
    printf("   %3d: (%f, %f, %f)\n", i,
        _g_ptr->value[0], _g_ptr->value[1], _g_ptr->value[2]);
  }


  ncanvas_t *ret_nptr;

  ret_nptr = ncv_alloc(ncv_cptr->width, ncv_cptr->height, RGB);
  if(ret_nptr != NULL) {
    for(i = 0; i < nof_cluster; ++i) {
      const size_t _offset = i * 3;

      _c_ptr = (clusters + i);
      _g_ptr = (gcenters + _c_ptr->id);

      memcpy((void *)(ret_nptr->data + _offset),
          (const void *)_g_ptr->value,
          sizeof(double) * 3);
    }
  }


  // 解放
  memset((void *)euclids, '\0', sizeof(double) * div_size);
  free((void *)euclids); euclids = NULL;

  memset((void *)pre_gcenters, '\0', sizeof(gcenter_t) * div_size);
  free((void *)pre_gcenters); pre_gcenters = NULL;

  memset((void *)gcenters, '\0', sizeof(gcenter_t) * div_size);
  free((void *)gcenters); gcenters = NULL;

  memset((void *)clusters, '\0', sizeof(cluster_t) * nof_cluster);
  free((void *)clusters); clusters = NULL;

  return ret_nptr;
}
