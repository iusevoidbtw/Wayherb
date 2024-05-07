#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>

NORETURN void die(const char *format, ...);
int create_tmpfile(off_t size);
int create_tmpfile_cloexec(char *tmpname);
int set_cloexec_or_close(int fd);
int strtouint(unsigned int *res, const char *s, int base);

#endif 
