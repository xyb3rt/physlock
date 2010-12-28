#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "physlock.h"
#include "options.h"

void print_usage() {
	printf("usage: physlock [-fhLlv]\n");
}

void print_version() {
	printf("physlock - lock all consoles\n");
	printf("Version %s, written by Bert Muennich\n", VERSION);
}

int parse_options(int argc, char **argv, options_t *options) {
	int opt;
	
	options->fg = 0;
	options->help = 0;
	options->o_lock = 0;
	options->o_unlock = 0;
	options->user = NULL;
	options->version = 0;

	while ((opt = getopt(argc, argv, "fhLlv")) != -1) {
		switch (opt) {
			case 'f':
				options->fg = 1;
				break;
			case 'h':
				options->help = 1;
				break;
			case 'L':
				options->o_unlock = 1;
				break;
			case 'l':
				options->o_lock = 1;
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
