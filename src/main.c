#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

#include "ppd_type.h"
#include "util.h"
#include "canvas/canvas.h"


bool_t f_color_correction = 0;

static void
print_help(const char *app);
static void
print_version(const char *app);

extern void
run_histogram(const canvas_t *cv_cptr);


int
main(int argc, char *argv[]) {
  const struct option longopts[] = {
    {"help",             no_argument, 0, 'H'}, // ヘルプ
    {"version",          no_argument, 0, 'V'}, // バージョン
    {"histogram",        no_argument, 0, 'h'},
    {"color-correction", no_argument, 0, 'c'}, // 色補正
    {0, 0, 0, 0},
  };
  
  int opt, longindex;

  bool_t f_histogram = FALSE;


  if(argc < 2) {
    fprintf(stderr, "%s: Operand Error.\n", argv[0]);
    return EOF;
  }

  while((opt = getopt_long(argc, argv, "HVhc",
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

      case 'c':
        f_color_correction = TRUE;
        break;
    }
  }


  canvas_t *target_ptr;
  target_ptr = cv_png_read(argv[argc - 1]);

  if(f_histogram == TRUE) {
    run_histogram(target_ptr);
  }
  
  cv_free(target_ptr);

  return 0;
}


static void
print_help(const char *app) {
  printf("Help\n");

  exit(0);
}

static void
print_version(const char *app) {
  printf("Version\n");

  exit(0);
}
