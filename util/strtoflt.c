/* See LICENSE file for copyright and license details. */

#include <errno.h>
#include <math.h>
#include <stdlib.h>

#define INVALID         1
#define TOOSMALL        2
#define TOOLARGE        3

/*
 * its kinda like strtonum but for floating point numbers,
 * i tried to make it very similar to make switching between
 * the two transparent
 */
double
strtoflt(const char *numstr, double minval, double maxval,
		const char **errstrp)
{
	double d = 0;
	int error = 0;
	char *ep;
	struct errval {
                const char *errstr;
                int err;
        } ev[4] = {
                { NULL,         0 },
                { "invalid",    EINVAL },
                { "too small",  ERANGE },
                { "too large",  ERANGE },
        };

	ev[0].err = errno;
	errno = 0;
	if (minval > maxval) {
		error = INVALID;
	} else {
		d = strtod(numstr, &ep);
		if (numstr == ep || *ep != '\0')
			error = INVALID;
		else if ((errno == ERANGE && d == HUGE_VAL) || d > maxval)
			error = TOOLARGE;
		else if (errno == ERANGE || d < minval)
			error = TOOSMALL;
	}
	if (errstrp != NULL)
		*errstrp = ev[error].errstr;
	errno = ev[error].err;
	if (error)
		d = 0.0;

	return d;
}
