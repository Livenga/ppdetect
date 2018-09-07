#ifndef _UTIL_H
#define _UTIL_H

#include "ppd_type.h"


/* src/util.c */
extern bool_t
is_exists(const char *path);
extern bool_t
is_directory(const char *path);

extern int
eprintf(FILE *strm, const char *fn, const char *param);
extern char *
get_datetime_s(void);


#endif
