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

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

const char *progname;

static FILE *Stderr;

void error_init(int fd)
{
	if ((fd = dup(fd)) == -1 || (Stderr = fdopen(fd, "w")) == NULL)
		Stderr = stderr;
}

void error(int eval, int err, const char* fmt, ...)
{
	va_list ap;

	fflush(stdout);
	fprintf(Stderr, "%s: ", progname);
	va_start(ap, fmt);
	if (fmt != NULL)
		vfprintf(Stderr, fmt, ap);
	va_end(ap);
	if (err != 0)
		fprintf(Stderr, "%s%s", fmt != NULL ? ": " : "", strerror(err));
	fputc('\n', Stderr);

	if (eval != 0)
		exit(eval);
}

char* estrdup(const char *s) {
	char *d;
	size_t n = strlen(s) + 1;

	d = malloc(n);
	if (d == NULL)
		error(EXIT_FAILURE, errno, NULL);
	memcpy(d, s, n);
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

	while ((ctl_file = fopen(path, "r")) == NULL && errno == EINTR);
	if (ctl_file == NULL)
		error(EXIT_FAILURE, errno, "%s", path);

	nread = fread(buf, 1, len - 1, ctl_file);
	if (ferror(ctl_file))
		error(EXIT_FAILURE, 0, "%s: Error reading file", path);

	fclose(ctl_file);
	buf[nread] = '\0';

	return nread;
}

/*
 * Write a buffer into a file.
 * Returns the number of characters written or -1 on failure.
 */
CLEANUP ssize_t write_file(const char *path, char *buf, size_t len) {
	FILE *ctl_file;
	size_t nwritten;

	while ((ctl_file = fopen(path, "w+")) == NULL && errno == EINTR);
	if (ctl_file == NULL) {
		error(0, errno, "%s", path);
		return -1;
	}

	nwritten = fwrite(buf, 1, len, ctl_file);
	if (ferror(ctl_file)) {
		error(0, 0, "%s: Error writing file", path);
		return -1;
	}
	fclose(ctl_file);

	return nwritten;
}

/*
 * Read integer from file, and ensure the next character is as expected.
 * The call always succeeds (it dies() on failure).
 */
int read_int_from_file(const char *path, char ending_char) {
	char buf[32], *end;
	int value;

	read_file(path, buf, sizeof(buf));

	value = strtol(buf, &end, 0);
	if (*end && *end != ending_char)
		error(EXIT_FAILURE, 0, "%s: Invalid file content", path);

	return value;
}

/*
 * Write integer to file.
 * Returns the number of characters written or -1 on failure.
 */
CLEANUP ssize_t write_int_to_file(const char *path, int value) {
	char buf[32];

	snprintf(buf, sizeof(buf), "%d\n", value);
	return write_file(path, buf, strlen(buf));
}

