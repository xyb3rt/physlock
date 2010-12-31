#ifndef PHYSLOCK_H
#define PHYSLOCK_H

#if __STDC_VERSION__ >= 199901L
#define _XOPEN_SOURCE 600
#else
#define _XOPEN_SOURCE 500
#endif

#include <stdlib.h>
#include <stdio.h>

#define VERSION "git-20101229"

#define CONSOLE_DEV "/dev/console"

#define WARN(...)                                                      \
	do {                                                                 \
	  fprintf(stderr, "physlock: %s:%d: warning: ", __FILE__, __LINE__); \
		fprintf(stderr, __VA_ARGS__);                                      \
		fprintf(stderr, "\n");                                             \
	} while (0)

#define FATAL(...)                                                     \
  do {                                                                 \
		fprintf(stderr, "physlock: %s:%d: error: ", __FILE__, __LINE__);   \
		fprintf(stderr, __VA_ARGS__);                                      \
		fprintf(stderr, "\n");                                             \
		cleanup();                                                         \
		exit(1);                                                           \
	} while (0)

void cleanup();

#endif /* PHYSLOCK_H */
