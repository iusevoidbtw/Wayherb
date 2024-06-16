/* See LICENSE file for copyright and license details. */

#ifndef UTIL_H
#define UTIL_H

#include <sys/types.h>

#include <stdio.h>

/* strtoflt.c */
double strtoflt(const char *numstr, double minval, double maxval,
		const char **errstrp);

/* util.c */
NORETURN void die(const char *format, ...);
int create_tmpfile(off_t size);

#endif 
