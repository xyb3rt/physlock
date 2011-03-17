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
#include <errno.h>

#define FNAME_LEN 512

int fd = -1;
char filename[FNAME_LEN];

void vt_init() {
	fd = open(CONSOLE_DEVICE, O_RDWR);
	if (fd < 0)
		DIE("could not open console device %s: %s", CONSOLE_DEVICE,
		      strerror(errno));
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
		DIE("get_current_vt() called without vt_init()");

	if (ioctl(fd, VT_GETSTATE, &vtstat) < 0)
		DIE("could not get state of active console: %s", strerror(errno));
	*nr = vtstat.v_active;

	if (user != NULL) {
		snprintf(filename, FNAME_LEN, TTY_DEVICE_BASE "%d", *nr);
		if (stat(filename, &fstat) < 0)
			DIE("could not stat file %s: %s", filename, strerror(errno));
		userinfo = getpwuid(fstat.st_uid);
		if (userinfo == NULL)
			DIE("could not get user info for uid %d", fstat.st_uid);
		*user = strdup(userinfo->pw_name);
		if (*user == NULL)
			DIE("could not allocate memory");
	}
}

void acquire_new_vt(vt_t *vt) {
	vt->nr = -1;
	vt->ios = NULL;
	vt->fd = -1;

	if (fd < 0)
		DIE("acquire_new_vt() called without vt_init()");
	if (ioctl(fd, VT_OPENQRY, &vt->nr) < 0)
		DIE("could not open new console: %s", strerror(errno));
	if (ioctl(fd, VT_ACTIVATE, vt->nr) < 0 ||
			ioctl(fd, VT_WAITACTIVE, vt->nr) < 0)
		DIE("could not activate console # %d: %s", vt->nr, strerror(errno));

	snprintf(filename, FNAME_LEN, TTY_DEVICE_BASE "%d", vt->nr);
	vt->ios = fopen(filename, "r+");
	if (vt->ios == NULL)
		DIE("could not open file %s: %s", filename, strerror(errno));
	vt->fd = fileno(vt->ios);

	tcgetattr(vt->fd, &vt->term);
	vt->rlflag = vt->term.c_lflag;
}

void release_vt(vt_t *vt, int nr) {
	if (fd < 0)
		DIE("release_vt() called without vt_init()");
	if (nr <= 0)
		DIE("release_vt() called with invalid argument");
	if (ioctl(fd, VT_ACTIVATE, nr) < 0 ||
			ioctl(fd, VT_WAITACTIVE, nr) < 0)
		DIE("could not activate console # %d: %s", vt->nr, strerror(errno));

	if (vt->ios != NULL) {
		fclose(vt->ios);
		vt->ios = NULL;
		vt->fd = -1;
	}

	if (vt->nr > 0) {
		if (ioctl(fd, VT_DISALLOCATE, vt->nr) < 0)
			DIE("could not deallocate console # %d: %s", vt->nr, strerror(errno));
		vt->nr = -1;
	}
}

void lock_vt_switch() {
	if (fd < 0)
		DIE("lock_vt_switch() called without vt_init()");
	if (ioctl(fd, VT_LOCKSWITCH, 1) < 0)
		DIE("could not lock console switching: %s", strerror(errno));
}

void unlock_vt_switch() {
	if (fd < 0)
		DIE("unlock_vt_switch() called without vt_init()");
	if (ioctl(fd, VT_UNLOCKSWITCH, 1) < 0)
		DIE("could not enable console switching: %s", strerror(errno));
}

void secure_vt(vt_t *vt) {
	if (vt->fd < 0)
		DIE("secure_vt() called with invalid argument");
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
		DIE("reset_vt() called with invalid argument");
	vt->term.c_lflag = vt->rlflag;
	tcsetattr(vt->fd, TCSANOW, &vt->term);
}
