/* sxiv: util.c
 * Copyright (c) 2013 Bert Muennich <be.muennich at gmail.com>
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

#define _POSIX_C_SOURCE 200112L
#define _XOPEN_SOURCE   500 /* for strdup() */

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

char* s_strdup(const char *s) {
	char *d = NULL;

	if (s != NULL) {
		d = malloc(strlen(s) + 1);
		if (d == NULL)
			die("could not allocate memory");
		strcpy(d, s);
	}
	return d;
}

/*
 * Read a file to a buffer.
 * The buffer is ensured to be NULL-terminated.
 * The call always succeeds (it dies() on failure).
 * Returns the number of characters read.
 */
size_t read_file(const char *path, char *buf, size_t len) {
	FILE *ctl_file;
	size_t nread;

	ctl_file = fopen(path, "r");
	if (ctl_file == NULL)
		die("could not open file: %s: %s", path, strerror(errno));

	nread = fread(buf, 1, len - 1, ctl_file);
	if (ferror(ctl_file))
		die("could not read file: %s: %s", path, strerror(errno));

	fclose(ctl_file);
	buf[nread] = '\0';

	return nread;
}

/*
 * Write a buffer into a file.
 * The call always succeeds (it dies() on failure).
 * Returns the number of characters written.
 */
size_t write_file(const char *path, char *buf, size_t len) {
	FILE *ctl_file;
	size_t nwritten;

	ctl_file = fopen(path, "w+");
	if (ctl_file == NULL)
		die("could not open file: %s: %s", path, strerror(errno));

	nwritten = fwrite(buf, 1, len, ctl_file);
	if (ferror(ctl_file))
		die("could not write file: %s: %s", path, strerror(errno));

	fclose(ctl_file);

	return nwritten;
}

/*
 * Read integer from file, and ensure the next character is as expected.
 * The call always succeeds (it dies() on failure).
 */
int read_int_from_file(const char *path, char ending_char) {
	char buf[BUFLEN], *end;
	int value;

	read_file(path, buf, BUFLEN);

	value = strtol(buf, &end, 0);
	if (*end && *end != ending_char)
		die("invalid file content: %s: %s", path, buf);

	return value;
}

/*
 * Write integer to file.
 * The call always succeeds (it dies() on failure).
 */
void write_int_to_file(const char *path, int value) {
	char buf[BUFLEN];

	snprintf(buf, BUFLEN, "%d\n", value);
	write_file(path, buf, strlen(buf));
}

