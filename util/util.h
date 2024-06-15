#ifndef UTIL_H
#define UTIL_H

#include <sys/types.h>

#include <stdio.h>

/* strtonum.c */
long long strtonum(const char *numstr, long long minval, long long maxval,
		const char **errstrp);

/* util.c */
NORETURN void die(const char *format, ...);
int create_tmpfile(off_t size);

#endif 
