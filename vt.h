/* physlock: vt.h
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

#ifndef VT_H
#define VT_H

#include <stdio.h>
#include <termios.h>

#define CLEANUP

typedef struct vt_s {
	int nr;
	FILE *ios;
	int fd;
	struct termios term;
	tcflag_t rlflag;
} vt_t;

void vt_init();
CLEANUP void vt_destroy();

void get_current_vt(int*);
CLEANUP int lock_vt_switch(int);

void acquire_new_vt(vt_t*);
void reopen_vt(vt_t*);
CLEANUP int release_vt(vt_t*, int);

void secure_vt(vt_t*);
void flush_vt(vt_t*);
CLEANUP void reset_vt(vt_t*);

#endif /* VT_H */
