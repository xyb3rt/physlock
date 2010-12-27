#ifndef PHYSLOCK_H
#define PHYSLOCK_H

#define CONSOLE_DEV "/dev/console"

#define WARN(msg)                                    \
	fprintf(stderr, "physlock: %s:%d: warning: %s\n",  \
			__FILE__, __LINE__, msg);

#define FATAL(msg)                                   \
  do {                                               \
		fprintf(stderr, "physlock: %s:%d: error: %s\n",  \
		    __FILE__, __LINE__, msg);                    \
		cleanup(0);                                      \
		exit(1);                                         \
	} while (0)

void cleanup(int warn);

#endif /* PHYSLOCK_H */
