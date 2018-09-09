#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "canvas/canvas.h"

canvas_t *
cv_draw_circuit_c(canvas_t *self,
    int x, int y,
    int width, int height,
    color_t color) {
  if(self == NULL || self->data == NULL) { return self; }


  double deg;

  for(deg = 0.0; deg <= 360.0; deg += 0.1) {
    const double _rad = (deg * M_PI) / 180.0;
    uint32_t _x, _y, _offset;

    _x      = (x +  ((double)width * cos(_rad) / 2.0));
    _y      = (y +  ((double)height* sin(_rad) / 2.0));
    if(_x < 0
        || _x >= self->width
        || _y < 0
        || _y >= self->height) { continue; }

    _offset = _y * self->width + _x;

    self->data[_offset * 3 + 0] = color.r;
    self->data[_offset * 3 + 1] = color.g;
    self->data[_offset * 3 + 2] = color.b;
  }

  return self;

}


canvas_t *
cv_draw_circuit(canvas_t *self,
    int x,     int y,
    int width, int height) {
  color_t c = { 0xFF, 0x00, 0x00 };
  return cv_draw_circuit_c(self, x, y, width, height, c);
}
