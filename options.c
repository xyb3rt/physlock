#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "physlock.h"
#include "options.h"

void print_usage() {
	printf("usage: physlock [-fhsv] [-u user]\n");
}

void print_version() {
	printf("physlock - lock all consoles\n");
	printf("version %s\n", VERSION);
}

int parse_arguments(int argc, char **argv, options_t *options) {
	int opt;
	
	options->fg = 0;
	options->help = 0;
	options->unlock = 0;
	options->user = NULL;
	options->version = 0;

	while ((opt = getopt(argc, argv, "fhsu:v")) != -1) {
		switch (opt) {
			case 'f':
				options->fg = 1;
				break;
			case 'h':
				options->help = 1;
				break;
			case 's':
				options->unlock = 1;
				break;
			case 'u':
				options->user = optarg;
				break;
			case 'v':
				options->version = 1;
				break;
		}
	}
	
	return 0;
}
