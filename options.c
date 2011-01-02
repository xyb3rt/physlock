/* physlock: options.c
 * Copyright (c) 2011 Bert Muennich <muennich at informatik.hu-berlin.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *  
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *  
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

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
	
	options->detach = 0;
	options->help = 0;
	options->only_lock = 0;
	options->only_unlock = 0;
	options->user = NULL;
	options->version = 0;

	while ((opt = getopt(argc, argv, "dhLlu:v")) != -1) {
		switch (opt) {
			case 'd':
				options->detach = 1;
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
