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

#include "options.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

options_t _options;
const options_t *options = (const options_t*) &_options;

void print_usage() {
	printf("usage: physlock [-dhLlsv] [-u user]\n");
}

void print_version() {
	printf("physlock %s - lock all consoles\n", VERSION);
}

void parse_options(int argc, char **argv) {
	int opt;
	
	_options.detach = 0;
	_options.disable_sysrq = 0;
	_options.only_lock = 0;
	_options.only_unlock = 0;
	_options.user = NULL;

	while ((opt = getopt(argc, argv, "dhLlsu:v")) != -1) {
		switch (opt) {
			case '?':
				print_usage();
				exit(1);
			case 'd':
				_options.detach = 1;
				break;
			case 'h':
				print_usage();
				exit(0);
			case 'L':
				_options.only_unlock = 1;
				break;
			case 'l':
				_options.only_lock = 1;
				break;
			case 's':
				_options.disable_sysrq = 1;
			case 'u':
				_options.user = optarg;
				break;
			case 'v':
				print_version();
				exit(0);
		}
	}
}
