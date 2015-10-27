/* sxiv: util.h
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

#ifndef UTIL_H
#define UTIL_H

#include <stdarg.h>
#include <sys/types.h>

#define CLEANUP

extern const char *progname;

void error(int, int, const char*, ...);

char* estrdup(const char*);

int read_int_from_file(const char*, char);
CLEANUP ssize_t write_int_to_file(const char*, int);

#endif /* UTIL_H */
