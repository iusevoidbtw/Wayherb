#define _POSIX_C_SOURCE 200809L
#include <sys/mman.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "util.h"

NORETURN
void
die(const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	putc('\n', stderr);
	va_end(ap);
	exit(1);
}

int
create_tmpfile(off_t size)
{
	static const char template[] = "/mayflower-shared-XXXXXX";
	const char *path;
	char *name;
	int fd;
	int ret;

	path = getenv("XDG_RUNTIME_DIR");
	if (!path) {
		errno = ENOENT;
		return -1;
	}

	name = malloc(strlen(path) + sizeof(template));
	if (!name) {
		return -1;
	}

	strcpy(name, path);
	strcat(name, template);

	fd = create_tmpfile_cloexec(name);
	free(name);

	if (fd < 0) {
		return -1;
	}

	ret = posix_fallocate(fd, 0, size);
	if (ret != 0) {
		close(fd);
		errno = ret;
		return -1;
	}
	
	return fd;
}

int
create_tmpfile_cloexec(char *tmpname)
{
	int fd;
#ifdef HAVE_MKOSTEMP
	fd = mkostemp(tmpname, O_CLOEXEC);

	if (fd >= 0) {
		unlink(tmpname);
	}
#else
	fd = mkstemp(tmpname);

	if (fd >= 0) {
		fd = set_cloexec_or_close(fd);
		unlink(tmpname);
	}
#endif
	return fd;
}

int
set_cloexec_or_close(int fd)
{
	int flags;

	if (fd == -1) {
		return -1;
	}

	flags = fcntl(fd, F_GETFD);

	if (flags == -1 || fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1) {
		close(fd);
		return -1;
	}
	return fd;
}

int
strtouint(unsigned int *res, const char *s, int base)
{
	/*
	 * safely convert the string s into an unsigned integer and store it in *res.
	 * returns 0 on success or -1 on error.
	 * if an error occurs, the contents of *res are unchanged.
	 */
	if (s == NULL || *s == '\0' || isspace(*s)) {
		fputs("bad number: bad string\n", stderr);
	} else if (res == NULL) {
		fputs("bad number: bad output pointer\n", stderr);
	} else if (!(isdigit(*s) || *s == '+' || *s == '-')) {
		fputs("bad number: extra characters at start of input\n", stderr);
	} else {
		char *end;
		long long l;
		errno = 0;
		l = strtoll(s, &end, base);
		if (l > UINT_MAX || (errno == ERANGE && l == LLONG_MAX)) {
			fputs("bad number: integer overflow\n", stderr);
		} else if (l < 0) {
			fputs("bad number: cannot be negative\n", stderr);
		} else if (errno == ERANGE && l == LLONG_MIN) {
			fputs("bad number: integer underflow\n", stderr);
		} else if (*end != '\0') {
			fputs("bad number: extra characters at end of input\n", stderr);
		} else {
			*res = (unsigned int)l;
			return 0;
		}
	}
	return -1;
}
