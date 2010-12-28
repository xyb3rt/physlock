#ifndef PHYSLOCK_H
#define PHYSLOCK_H

#define CONSOLE_DEV "/dev/console"

#define WARN(msg)                                                      \
	do {                                                                 \
		fprintf(stderr, "physlock: %s:%d: warning: ", __FILE__, __LINE__); \
		perror(msg);                                                       \
	} while (0)

#define FATAL(msg)                                                     \
  do {                                                                 \
		fprintf(stderr, "physlock: %s:%d: error: ", __FILE__, __LINE__);   \
		perror(msg);                                                       \
		cleanup(0);                                                        \
		exit(1);                                                           \
	} while (0)

void cleanup(int warn);

#endif /* PHYSLOCK_H */
