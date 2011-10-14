/* sxiv: util.c
 * Copyright (c) 2011 Bert Muennich <be.muennich at googlemail.com>
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "util.h"

enum { BUFLEN = 32 };

void cleanup();

void warn(const char *fmt, ...) {
	va_list args;

	if (!fmt)
		return;

	va_start(args, fmt);
	fprintf(stderr, "physlock: warning: ");
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
	va_end(args);
}

void die(const char *fmt, ...) {
	va_list args;

	if (!fmt)
		return;

	va_start(args, fmt);
	fprintf(stderr, "physlock: error: ");
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
	va_end(args);

	cleanup();
	exit(1);
}

int get_sysrq_state(const char *path) {
	char buf[BUFLEN], *end;
	int len, state;
	FILE *ctl_file;

	if (!path)
		return -1;

	ctl_file = fopen(path, "r");
	if (ctl_file == NULL)
		die("could not open file: %s", path);

	len = fread(buf, 1, BUFLEN - 1, ctl_file);
	if (ferror(ctl_file))
		die("could not read file: %s: %s", path, strerror(errno));

	fclose(ctl_file);

	buf[len] = '\0';
	state = strtol(buf, &end, 0);
	if (*end && *end != '\n')
		die("invalid file content: %s: %s", path, buf);

	return state;
}

void set_sysrq_state(const char *path, int new_state) {
	char buf[BUFLEN];
	FILE *ctl_file;

	if (!path)
		return;

	ctl_file = fopen(path, "w+");
	if (ctl_file == NULL)
		die("could not open file: %s", path);

	snprintf(buf, BUFLEN, "%d\n", new_state);

	fwrite(buf, 1, strlen(buf), ctl_file);
	if (ferror(ctl_file))
		die("could not write file: %s: %s", path, strerror(errno));

	fclose(ctl_file);
}
