#include "physlock.h"
#include "options.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

void print_usage() {
	printf("usage: physlock [-dhLlv] [-u user]\n");
}

void print_version() {
	printf("physlock - lock all consoles\n");
	printf("Version %s, written by Bert Muennich\n", VERSION);
}

int parse_options(int argc, char **argv, options_t *options) {
	int opt;
	
	options->bg = 0;
	options->help = 0;
	options->only_lock = 0;
	options->only_unlock = 0;
	options->user = NULL;
	options->version = 0;

	while ((opt = getopt(argc, argv, "dhLlu:v")) != -1) {
		switch (opt) {
			case 'd':
				options->bg = 1;
				break;
			case 'h':
				options->help = 1;
				break;
			case 'L':
				options->only_unlock = 1;
				break;
			case 'l':
				options->only_lock = 1;
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
