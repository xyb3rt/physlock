/* physlock: vt.c
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
#include "vt.h"

#include <fcntl.h>
#include <pwd.h>
#include <string.h>
#include <linux/vt.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/unistd.h>
#include <sys/ioctl.h>

#define FNAME_LEN 512

int fd = -1;
char filename[FNAME_LEN];

void vt_init() {
	fd = open(CONSOLE_DEV, O_RDWR);
	if (fd < 0)
		FATAL("could not open console device %s", CONSOLE_DEV);
}

void vt_destroy() {
	if (fd >= 0) {
		close(fd);
		fd = -1;
	}
}

void get_current_vt(int *nr, const char **user) {
	struct stat fstat;
	struct vt_stat vtstat;
	struct passwd *userinfo;

	if (fd < 0)
		FATAL("get_current_vt() called without vt_init()");

	if (ioctl(fd, VT_GETSTATE, &vtstat) < 0)
		FATAL("could not get state of active console");
	*nr = vtstat.v_active;

	if (user != NULL) {
		snprintf(filename, FNAME_LEN, "/dev/tty%d", *nr);
		if (stat(filename, &fstat) < 0)
			FATAL("could not stat file %s", filename);
		userinfo = getpwuid(fstat.st_uid);
		if (userinfo == NULL)
			FATAL("could not get user info for uid %d", fstat.st_uid);
		*user = strdup(userinfo->pw_name);
		if (*user == NULL)
			FATAL("could not allocate memory");
	}
}

void acquire_new_vt(vt_t *vt) {
	vt->nr = -1;
	vt->ios = NULL;
	vt->fd = -1;

	if (fd < 0)
		FATAL("acquire_new_vt() called without vt_init()");
	if (ioctl(fd, VT_OPENQRY, &vt->nr) < 0)
		FATAL("could not open new console");
	if (ioctl(fd, VT_ACTIVATE, vt->nr) < 0 ||
			ioctl(fd, VT_WAITACTIVE, vt->nr) < 0)
		FATAL("could not activate console # %d", vt->nr);

	snprintf(filename, FNAME_LEN, "/dev/tty%d", vt->nr);
	vt->ios = fopen(filename, "r+");
	if (vt->ios == NULL)
		FATAL("could not open file %s", filename);
	vt->fd = fileno(vt->ios);

	tcgetattr(vt->fd, &vt->term);
	vt->rlflag = vt->term.c_lflag;
}

void release_vt(vt_t *vt, int nr) {
	if (fd < 0)
		FATAL("release_vt() called without vt_init()");
	if (nr <= 0)
		FATAL("release_vt() called with invalid argument");
	if (ioctl(fd, VT_ACTIVATE, nr) < 0 ||
			ioctl(fd, VT_WAITACTIVE, nr) < 0)
		FATAL("could not activate console # %d", vt->nr);

	if (vt->ios != NULL) {
		fclose(vt->ios);
		vt->ios = NULL;
		vt->fd = -1;
	}

	if (vt->nr > 0) {
		if (ioctl(fd, VT_DISALLOCATE, vt->nr) < 0)
			FATAL("could not deallocate console # %d", vt->nr);
		vt->nr = -1;
	}
}

void lock_vt_switch() {
	if (fd < 0)
		FATAL("lock_vt_switch() called without vt_init()");
	if (ioctl(fd, VT_LOCKSWITCH, 1) < 0)
		FATAL("could not lock console switching");
}

void unlock_vt_switch() {
	if (fd < 0)
		FATAL("unlock_vt_switch() called without vt_init()");
	if (ioctl(fd, VT_UNLOCKSWITCH, 1) < 0)
		FATAL("could not enable console switching");
}

void secure_vt(vt_t *vt) {
	if (vt->fd < 0)
		FATAL("secure_vt() called with invalid argument");
	vt->term.c_lflag &= ~(ECHO | ISIG);
	tcsetattr(vt->fd, TCSANOW, &vt->term);
}

void tty_break_on(vt_t *vt) {
	if (vt->fd < 0) {
		WARN("tty_break_on() called with invalid argument");
		return;
	}
	vt->term.c_lflag &= ~ICANON;
	tcsetattr(vt->fd, TCSANOW, &vt->term);
}

void tty_break_off(vt_t *vt) {
	if (vt->fd < 0) {
		WARN("tty_break_off() called with invalid argument");
		return;
	}
	vt->term.c_lflag |= ICANON;
	tcsetattr(vt->fd, TCSANOW, &vt->term);
}

void reset_vt(vt_t *vt) {
	if (vt->fd < 0)
		FATAL("reset_vt() called with invalid argument");
	vt->term.c_lflag = vt->rlflag;
	tcsetattr(vt->fd, TCSANOW, &vt->term);
}
