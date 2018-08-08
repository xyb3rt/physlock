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
#include "config.h"

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/vt.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <errno.h>

static int fd = -1;
static char filename[1024];

void vt_init() {
	while ((fd = open(CONSOLE_DEVICE, O_RDWR)) == -1 && errno == EINTR);
	if (fd < 0)
		error(EXIT_FAILURE, errno, "%s", CONSOLE_DEVICE);
}

CLEANUP void vt_destroy() {
	if (fd >= 0) {
		close(fd);
		fd = -1;
	}
}

void vt_get_current(int *nr, uid_t *owner) {
	int ret;
	struct stat fstat;
	struct vt_stat vtstat;

	while ((ret = ioctl(fd, VT_GETSTATE, &vtstat)) == -1 && errno == EINTR);
	if (ret == -1)
		error(EXIT_FAILURE, errno, "%s: VT_GETSTATE", CONSOLE_DEVICE);
	*nr = vtstat.v_active;
	snprintf(filename, sizeof(filename), "%s%d", TTY_DEVICE_BASE, *nr);
	*owner = stat(filename, &fstat) == 0 ? fstat.st_uid : (uid_t)-1;
}

CLEANUP int vt_lock_switch(int set) {
	int ret;
	
	if (set) {
		while ((ret = ioctl(fd, VT_LOCKSWITCH, 1)) == -1 && errno == EINTR);
		if (ret == -1)
			error(0, errno, "%s: VT_LOCKSWITCH", CONSOLE_DEVICE);
	} else {
		while ((ret = ioctl(fd, VT_UNLOCKSWITCH, 1)) == -1 && errno == EINTR);
		if (ret == -1)
			error(0, errno, "%s: VT_UNLOCKSWITCH", CONSOLE_DEVICE);
	}
	return ret;
}

void vt_acquire(vt_t *vt) {
	int ret;

	vt->nr = -1;
	vt->ios = NULL;
	vt->fd = -1;

	while ((ret = ioctl(fd, VT_OPENQRY, &vt->nr)) == -1 && errno == EINTR);
	if (ret == -1)
		error(EXIT_FAILURE, errno, "%s: VT_OPENQRY", CONSOLE_DEVICE);

	snprintf(filename, sizeof(filename), "%s%d", TTY_DEVICE_BASE, vt->nr);
	while ((vt->ios = fopen(filename, "r+")) == NULL && errno == EINTR);
	if (vt->ios == NULL)
		error(EXIT_FAILURE, errno, "%s", filename);
	vt->fd = fileno(vt->ios);

	while ((ret = ioctl(fd, VT_ACTIVATE, vt->nr)) == -1 && errno == EINTR);
	if (ret == -1)
		error(EXIT_FAILURE, errno, "%s: VT_ACTIVATE", CONSOLE_DEVICE);
	while ((ret = ioctl(fd, VT_WAITACTIVE, vt->nr)) == -1 && errno == EINTR);
	if (ret == -1)
		error(EXIT_FAILURE, errno, "%s: VT_WAITACTIVE", CONSOLE_DEVICE);

	tcgetattr(vt->fd, &vt->term);
	memcpy(&vt->term_orig, &vt->term, sizeof(vt->term_orig));
}

void vt_reopen(vt_t *vt) {
	FILE *ios = vt->ios;

	vt->fd = -1;
	while ((vt->ios = freopen(filename, "r+", ios)) == NULL && errno == EINTR);
	if (vt->ios == NULL)
		error(EXIT_FAILURE, errno, "%s", filename);
	vt->fd = fileno(vt->ios);
}

CLEANUP int vt_release(vt_t *vt, int nr) {
	int ret;

	while ((ret = ioctl(fd, VT_ACTIVATE, nr)) == -1 && errno == EINTR);
	if (ret == -1) {
		error(0, errno, "%s: VT_ACTIVATE", CONSOLE_DEVICE);
		return -1;
	}
	while ((ret = ioctl(fd, VT_WAITACTIVE, nr)) == -1 && errno == EINTR);
	if (ret == -1) {
		error(0, errno, "%s: VT_WAITACTIVE", CONSOLE_DEVICE);
		return -1;
	}

	if (vt->ios != NULL) {
		fclose(vt->ios);
		vt->ios = NULL;
		vt->fd = -1;
	}

	if (vt->nr > 0) {
		while ((ret = ioctl(fd, VT_DISALLOCATE, vt->nr)) == -1 && errno == EINTR);
		if (ret == -1) {
			error(0, errno, "%s: VT_DISALLOCATE", CONSOLE_DEVICE);
			return -1;
		}
		vt->nr = -1;
	}
	return 0;
}

void vt_secure(vt_t *vt) {
	vt->term.c_iflag &= ~IXON;
	vt->term.c_lflag &= ~(ECHO | ISIG);
	tcsetattr(vt->fd, TCSANOW, &vt->term);
}

CLEANUP void vt_reset(vt_t *vt) {
	fprintf(vt->ios, "\033[H\033[J"); /* clear the screen */
	tcsetattr(vt->fd, TCSANOW, &vt->term_orig);
}

