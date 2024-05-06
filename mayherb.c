#define _POSIX_C_SOURCE
#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "config.h"
#include "util.h"
#include "wayland.h"

static volatile sig_atomic_t exit_code = 2;
static volatile sig_atomic_t should_exit = 0;

void
expire(int sig)
{
	switch (sig) {
		case SIGUSR2:
			exit_code = EXIT_SUCCESS;
			should_exit = 1;
			break;
		case SIGUSR1:
		case SIGALRM:
			should_exit = 1;
			break;
	}
}

void
time_set(struct timespec *t, uint64_t seconds, uint64_t nanosec)
{
	t->tv_sec = seconds;
	t->tv_nsec = nanosec;
}

void
time_elapsed(struct timespec *c, const struct timespec *a, const struct timespec *b)
{
	c->tv_sec = a->tv_sec - b->tv_sec;
	if (b->tv_nsec > a->tv_nsec) {
		c->tv_sec--;
		c->tv_nsec = 1000000000;
		c->tv_nsec += a->tv_nsec;
		c->tv_nsec -= b->tv_nsec;
	} else {
		c->tv_nsec = a->tv_nsec - b->tv_nsec;
	}
}

int
time_lessthan(const struct timespec *a, const struct timespec *b)
{
	return a->tv_sec == b->tv_sec ?
		a->tv_nsec < b->tv_nsec :
		a->tv_sec < b->tv_sec;
}

char *
concat(int argc, char *argv[])
{
	if (argc <= 1) {
		return NULL;
	}
	size_t sz = 0;
	int i;
	for (i = 1; i < argc; i++) {
		sz += strlen(argv[i]);
		if (i + 1 < argc) {
			/* + a space */
			sz++;
		}
	}
	if (sz > 0) {
		char *s = calloc(sz + 1, sizeof(char)); /* one for the null terminator */
		char *end;
		if (s == NULL) {
			die("calloc failed\n");
		}

		/*
		 * note that a buffer overflow shouldn't happen
		 * since the string we just allocated is guaranteed
		 * to be large enough to hold everything
		 */
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

int
main(int argc, char *argv[])
{
	if (argc == 0) {
		/* ISO C11 explicitly states that argc can be 0 (5.1.2.2.1 Program startup) */
		sem_unlink("/mayherb");
		die("argc == 0\n");
	}

	if (argc == 1) {
		sem_unlink("/mayherb");
		die("usage: %s string ...", argv[0]);
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
	
	sem_t *mutex = sem_open("/mayherb", O_CREAT, 0644, 1);
	sem_wait(mutex);
	
	sigaction(SIGUSR1, &act_expire, 0);
	sigaction(SIGUSR2, &act_expire, 0);
	
	struct timespec last_frame;
	struct timespec current_frame;
	struct timespec frame_delta;
	struct timespec sleep_time;
	struct timespec limit;

	memset(&last_frame, 0, sizeof(struct timespec));
	memset(&current_frame, 0, sizeof(struct timespec));
	memset(&frame_delta, 0, sizeof(struct timespec));
	memset(&sleep_time, 0, sizeof(struct timespec));
	memset(&limit, 0, sizeof(struct timespec));

	time_set(&limit, 0, 40000000);

	clock_settime(CLOCK_MONOTONIC_RAW, &last_frame);
	current_frame = last_frame;

	char *s = concat(argc, argv);
	if (s == NULL) {
		return EXIT_FAILURE;
	}
	init_wayland(s);

	if (duration != 0) {
		alarm(duration);
	}

	while (!should_exit) {
		draw();

		last_frame = current_frame;
		clock_settime(CLOCK_MONOTONIC_RAW, &current_frame);
		time_elapsed(&frame_delta, &current_frame, &last_frame);
		if (time_lessthan(&frame_delta, &limit)) {
			time_elapsed(&sleep_time, &limit, &frame_delta);
			nanosleep(&sleep_time, NULL);
		}
	}

	sem_post(mutex);
	sem_close(mutex);
	quit_wayland();
	free(s);
	return (int)exit_code;
}
