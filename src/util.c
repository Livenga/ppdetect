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


static char _str_dt[128], _name_buf[1024];

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

char *
get_output_filename(const char *target, const char *attr, const char *extend) {
  char *p_str, *p_end, *p_extend;
  char *p_name;


  memset((void *)_name_buf, '\0', sizeof(_name_buf));

  p_end = NULL;
  for(p_str = strchr(target, '/'); p_str != NULL; p_str = strchr(p_end, '/')) {
    p_end = p_str + 1;
  }

  p_str    = (p_end    != NULL) ? p_end : (char *)target;
  p_extend = strchr(p_str, '.');
  const size_t name_len = (p_extend != NULL) ? (p_extend - p_str) : strlen(p_str);

  p_name = (char *)calloc(name_len + 1, sizeof(char));
  memcpy((void *)p_name, (const void *)p_str, sizeof(char) * name_len);

  size_t path_length;
  path_length  = name_len + strlen(extend) + (attr == NULL ? 0 : strlen(attr)) + 25;

  if(path_length < 1024) {
    if(attr != NULL) {
      sprintf(_name_buf, "%s_%s_%s.%s", get_datetime_s(), p_name, attr, extend);
    } else {
      sprintf(_name_buf, "%s_%s.%s", get_datetime_s(), p_name, extend);
    }
  } else {
    sprintf(_name_buf, "output.%s", extend);
  }

  // *p_name 解放
  memset((void *)p_name, '\0', sizeof(char) * name_len);
  free((void *)p_name); p_name = NULL;

  return (char *)_name_buf;
}


int
eprintf(FILE *strm, const char *fn, const char *param) {
  if(param != NULL) {
    return fprintf(strm, "%s: %s %s\n", fn, param, strerror(errno));
  }
  return fprintf(strm, "%s: %s\n", fn, strerror(errno));
}
