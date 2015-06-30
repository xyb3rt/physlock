/* physlock: vt.c
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

#include <fcntl.h>
#include <pwd.h>
#include <string.h>
#include <unistd.h>
#include <linux/vt.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "config.h"
#include "util.h"
#include "vt.h"

enum { FNAME_LEN = 1024 };

static int fd = -1;
static char filename[FNAME_LEN];

void vt_init() {
	fd = open(CONSOLE_DEVICE, O_RDWR);
	if (fd < 0)
		die("could not open console device %s: %s", CONSOLE_DEVICE,
		      strerror(errno));
}

void vt_destroy() {
	if (fd >= 0) {
		close(fd);
		fd = -1;
	}
}

void get_current_vt(int *nr) {
	struct vt_stat vtstat;

	if (fd < 0)
		die("get_current_vt() called without vt_init()");

	if (ioctl(fd, VT_GETSTATE, &vtstat) == -1)
		die("could not get state of active console: %s", strerror(errno));
	*nr = vtstat.v_active;
}

void acquire_new_vt(vt_t *vt) {
	int ret;

	vt->nr = -1;
	vt->ios = NULL;
	vt->fd = -1;

	if (fd < 0)
		die("acquire_new_vt() called without vt_init()");
	if (ioctl(fd, VT_OPENQRY, &vt->nr) == -1)
		die("could not open new console: %s", strerror(errno));

	snprintf(filename, FNAME_LEN, "%s%d", TTY_DEVICE_BASE, vt->nr);
	vt->ios = fopen(filename, "r+");
	if (vt->ios == NULL)
		die("could not open %s: %s", filename, strerror(errno));
	vt->fd = fileno(vt->ios);

	if (ioctl(fd, VT_ACTIVATE, vt->nr) == -1)
		die("could not activate console # %d: %s", vt->nr, strerror(errno));
	while ((ret = ioctl(fd, VT_WAITACTIVE, vt->nr)) == -1 && errno == EINTR);
	if (ret == -1)
		die("could not wait for console # %d: %s", vt->nr, strerror(errno));

	tcgetattr(vt->fd, &vt->term);
	vt->rlflag = vt->term.c_lflag;
}

void reopen_vt(vt_t *vt) {
	if (vt->nr < 0)
		die("reopen_vt() called without acquire_new_vt()");
	vt->fd = -1;
	vt->ios = freopen(filename, "r+", vt->ios);
	if (vt->ios == NULL)
		die("could not open %s: %s", filename, strerror(errno));
	vt->fd = fileno(vt->ios);
}

void release_vt(vt_t *vt, int nr) {
	int ret;

	if (fd < 0)
		die("release_vt() called without vt_init()");
	if (nr <= 0)
		die("release_vt() called with invalid argument");

	if (ioctl(fd, VT_ACTIVATE, nr) == -1)
		die("could not activate console # %d: %s", nr, strerror(errno));
	while ((ret = ioctl(fd, VT_WAITACTIVE, nr)) == -1 && errno == EINTR);
	if (ret == -1)
		die("could not wait for console # %d: %s", nr, strerror(errno));

	if (vt->ios != NULL) {
		fclose(vt->ios);
		vt->ios = NULL;
		vt->fd = -1;
	}

	if (vt->nr > 0) {
		if (ioctl(fd, VT_DISALLOCATE, vt->nr) == -1)
			die("could not deallocate console # %d: %s", vt->nr, strerror(errno));
		vt->nr = -1;
	}
}

void lock_vt_switch() {
	if (fd < 0)
		die("lock_vt_switch() called without vt_init()");
	if (ioctl(fd, VT_LOCKSWITCH, 1) == -1)
		die("could not lock console switching: %s", strerror(errno));
}

void unlock_vt_switch() {
	if (fd < 0)
		die("unlock_vt_switch() called without vt_init()");
	if (ioctl(fd, VT_UNLOCKSWITCH, 1) == -1)
		die("could not enable console switching: %s", strerror(errno));
}

void secure_vt(vt_t *vt) {
	if (vt->fd < 0)
		die("secure_vt() called with invalid argument");
	vt->term.c_lflag &= ~(ECHO | ISIG);
	tcsetattr(vt->fd, TCSANOW, &vt->term);
}

void tty_echo_on(vt_t *vt) {
	if (vt->fd < 0) {
		warn("tty_break_on() called with invalid argument");
		return;
	}
	vt->term.c_lflag |= ECHO;
	tcsetattr(vt->fd, TCSANOW, &vt->term);
}

void tty_echo_off(vt_t *vt) {
	if (vt->fd < 0) {
		warn("tty_break_off() called with invalid argument");
		return;
	}
	vt->term.c_lflag &= ~ECHO;
	tcsetattr(vt->fd, TCSANOW, &vt->term);
}

void flush_vt(vt_t *vt) {
	if (vt->fd >= 0)
		tcflush(vt->fd, TCIFLUSH);
}

void reset_vt(vt_t *vt) {
	if (vt->fd < 0)
		die("reset_vt() called with invalid argument");

	/* clear the screen: */
	fprintf(vt->ios, "\033[H\033[J");

	vt->term.c_lflag = vt->rlflag;
	tcsetattr(vt->fd, TCSANOW, &vt->term);
}
