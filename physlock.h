/* physlock: physlock.h
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

#ifndef PHYSLOCK_H
#define PHYSLOCK_H

#if __STDC_VERSION__ >= 199901L
#define _XOPEN_SOURCE 600
#else
#define _XOPEN_SOURCE 500
#endif

#include <stdlib.h>
#include <stdio.h>

#include "config.h"

#define WARN(...)                                                      \
	do {                                                                 \
	  fprintf(stderr, "physlock: %s:%d: warning: ", __FILE__, __LINE__); \
		fprintf(stderr, __VA_ARGS__);                                      \
		fprintf(stderr, "\n");                                             \
	} while (0)

#define FATAL(...)                                                     \
  do {                                                                 \
		fprintf(stderr, "physlock: %s:%d: error: ", __FILE__, __LINE__);   \
		fprintf(stderr, __VA_ARGS__);                                      \
		fprintf(stderr, "\n");                                             \
		cleanup();                                                         \
		exit(1);                                                           \
	} while (0)

void cleanup();

#endif /* PHYSLOCK_H */
