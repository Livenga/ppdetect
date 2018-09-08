#ifndef _CALC_TYPE_H
#define _CALC_TYPE_H

#include <stdint.h>


typedef enum {
  nR = 0, nG, nB
} cc_id_t;


typedef struct {
  uint8_t id;
  double value[3];
} cluster_t;


typedef struct {
  uint32_t count;
  double value[3];
} gcenter_t;

#endif
