/* See LICENSE file for copyright and license details. */

#ifndef UTIL_H
#define UTIL_H

#include "config.h"

/* strtoflt.c */
double strtoflt(const char *numstr, double minval, double maxval,
		const char **errstrp);

/* die.c */
NORETURN void die(const char *fmt, ...);

#endif 
