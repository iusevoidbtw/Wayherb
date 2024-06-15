#define _POSIX_C_SOURCE
#include <sys/stat.h>

#include <errno.h>
#include <limits.h>
#include <semaphore.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "draw.h"
#include "util/util.h"

static volatile sig_atomic_t exit_code = EXIT_DISMISS;
static volatile sig_atomic_t should_exit = 0;

static void
expire(int sig)
{
	switch (sig) {
	case SIGUSR2:
		exit_code = 0;
		should_exit = 1;
		break;
	case SIGUSR1:
	case SIGALRM:
		should_exit = 1;
		break;
	}
}

static char *
concat(int argc, char *argv[])
{
	if (argc <= 1)
		return NULL;

	size_t sz = 0;
	int i;
	for (i = 1; i < argc; i++) {
		sz += strlen(argv[i]);
		if (i + 1 < argc) {
			/* + a space */
			++sz;
		}
	}
	if (sz > 0) {
		char *s = malloc(sz + 1);
		char *end;
		if (!s)
			die("malloc: out of memory");

		/*
		 * note that a buffer overflow shouldn't happen
		 * since the string we just allocated is guaranteed
		 * to be large enough to hold everything
		 */
		s[0] = '\0';
		for (i = 1; i < argc; i++) {
			strcat(s, argv[i]);
			if (i + 1 < argc) {
				end = s + strlen(s);
				*end = ' ';
				*(end + 1) = '\0';
			}
		}
		return s;
	}
	return NULL;
}

NORETURN static inline void
help(const char *argv0)
{
	sem_unlink("/mayflower");
	die("usage: %s [OPTION] STRING ...\n"
		"displays STRING and (optionally) any following arguments concatenated together\n"
		"in a notification window."
		"\n\n"
		"possible OPTIONs are:\n"
		"-d SECONDS        --duration SECONDS        sets the timeout before the notification is automatically\n"
		"                                                dismissed to SECONDS seconds; set to 0 to disable\n"
		"                                                auto-dismiss\n"
		"--help                                      print this message and exit", argv0);
}

NORETURN static inline void
usage(const char *argv0)
{
	sem_unlink("/mayflower");
	die("%s: use '%s --help' for usage instructions", argv0, argv0);
}

int
main(int argc, char *argv[])
{
	if (argc == 0) {
		/* ISO C11 explicitly states that argc can be 0 (5.1.2.2.1 Program startup) */
		sem_unlink("/mayflower");
		die("argc == 0");
	}

	if (argc == 1)
		usage(argv[0]);

	int i = 1;
        for (; i < argc; i++) {
                if (strcmp(argv[i], "--") == 0) {
			++i;
                        break;
		} else if (strcmp(argv[i], "--help") == 0) {
			help(argv[0]);
                } else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--duration") == 0) {
			if (i + 1 < argc) {
				const char *errstr;
				duration = (unsigned int)strtonum(argv[++i], 0, UINT_MAX, &errstr);
				if (errstr) {
					sem_unlink("/mayflower");
					die("converting string '%s' to integer: %s", argv[i], errstr);
				}
			} else {
				sem_unlink("/mayflower");
				die("%s: missing required argument for option '%s'", argv[0], argv[i]);
			}
                } else if (argv[i][0] == '-') {
			usage(argv[0]);
		} else {
			break;
		}
	}

	struct sigaction act_expire, act_ignore;
	act_expire.sa_handler = expire;
	act_expire.sa_flags = SA_RESTART;
	sigemptyset(&act_expire.sa_mask);
	
	act_ignore.sa_handler = SIG_IGN;
	act_ignore.sa_flags = SA_RESTART;
	sigemptyset(&act_ignore.sa_mask);
	
	sigaction(SIGALRM, &act_expire, 0);
	sigaction(SIGTERM, &act_expire, 0);
	sigaction(SIGINT, &act_expire, 0);
	
	sigaction(SIGUSR1, &act_ignore, 0);
	sigaction(SIGUSR2, &act_ignore, 0);
	
	sem_t *mutex = sem_open("/mayflower", O_CREAT, 0644, 1);
	sem_wait(mutex);
	
	sigaction(SIGUSR1, &act_expire, 0);
	sigaction(SIGUSR2, &act_expire, 0);
	
	char *s = concat(argc - i + 1, argv + i - 1);
	if (!s)
		return 1;
	init_draw(s);

	if (duration != 0)
		alarm(duration);

	while (!should_exit)
		dispatch();

	sem_post(mutex);
	sem_close(mutex);
	quit_draw();
	free(s);
	return (int)exit_code;
}
