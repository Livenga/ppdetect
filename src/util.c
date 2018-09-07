#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ppd_type.h"


bool_t
is_exists(const char *path) {
  if(path == NULL) { return FALSE; }
  return (access(path, F_OK) == 0 ? TRUE : FALSE);
}

bool_t
is_directory(const char *path) {
  struct stat statbuf;

  if(is_exists(path) == FALSE) {
    return FALSE;
  }

  memset((void *)&statbuf, '\0', sizeof(struct stat));
  if(lstat(path, &statbuf) < 0) {
    return FALSE;
  }

  return ((S_ISDIR(statbuf.st_mode) > 0) ? TRUE : FALSE);
}


static char _str_dt[128];

char *
get_datetime_s(void) {
  time_t t;
  struct tm *tm;


  memset((void *)_str_dt, '\0', sizeof(_str_dt));
  memset((void *)&tm,    '\0', sizeof(struct tm));

  t = time(NULL);
  tm = localtime((const time_t *)&t);

  sprintf(_str_dt, "%04d-%02d-%02dT%02d:%02d:%02d",
      (1900 + tm->tm_year), (1 + tm->tm_mon), tm->tm_mday,
      tm->tm_hour, tm->tm_min, tm->tm_sec);

  return (char *)_str_dt;
}


int
eprintf(FILE *strm, const char *fn, const char *param) {
  if(param != NULL) {
    return fprintf(strm, "%s: %s %s\n", fn, param, strerror(errno));
  }
  return fprintf(strm, "%s: %s\n", fn, strerror(errno));
}
