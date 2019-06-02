/* Copyright 2013 Bert Muennich
 *
 * This file is part of physlock.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "physlock.h"
#include "version.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static options_t _options;
const options_t *options = (const options_t*) &_options;

void print_usage() {
	printf("usage: physlock [-dhLlmsv] [-p MSG]\n");
}

void print_version() {
	puts("physlock " VERSION);
}

void parse_options(int argc, char **argv) {
	int opt;

	progname = strrchr(argv[0], '/');
	progname = progname != NULL ? progname + 1 : argv[0];

	_options.detach = 0;
	_options.disable_sysrq = 0;
	_options.lock_switch = -1;
	_options.mute_kernel_messages = 0;
	_options.no_auth = 0;

	while ((opt = getopt(argc, argv, "dhLlmnp:svb:a:")) != -1) {
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
				_options.lock_switch = 0;
				break;
			case 'l':
				_options.lock_switch = 1;
				break;
			case 'm':
				_options.mute_kernel_messages = 1;
				break;
			case 'n':
				_options.no_auth = 1;
				break;
			case 'p':
				_options.prompt = optarg;
				break;
			case 'b':
				_options.command_before = optarg;
				break;
			case 'a':
				_options.command_after = optarg;
				break;
			case 's':
				_options.disable_sysrq = 1;
				break;
			case 'v':
				print_version();
				exit(0);
		}
	}
}

