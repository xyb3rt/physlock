#ifndef OPTIONS_H
#define OPTIONS_H

typedef struct options_s {
	int fg;
	int help;
	int unlock;
	const char *user;
	int version;
} options_t;

void print_usage();
void print_version();

int parse_arguments(int, char**, options_t*);

#endif /* OPTIONS_H */
