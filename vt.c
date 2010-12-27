#include <stdlib.h>
#include <fcntl.h>
#include <pwd.h>
#include <string.h>
#include <termios.h>
#include <linux/vt.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/unistd.h>
#include <sys/ioctl.h>

#include "physlock.h"
#include "vt.h"

#define FNAME_LEN 512

int fd;
char filename[FNAME_LEN];

int vt_init() {
	fd = open(CONSOLE_DEV, O_RDWR);
	if (fd < 0)
		return -1;
	return 0;
}

void vt_destroy() {
	if (fd >= 0) {
		close(fd);
		fd = -1;
	}
}

int get_current_vt(int *nr, char **user) {
	struct stat fstat;
	struct vt_stat vtstat;
	struct passwd *userinfo;
	int err;

	if (fd < 0)
		return -1;
	err = ioctl(fd, VT_GETSTATE, &vtstat);
	if (err < 0)
		return err;
	*nr = vtstat.v_active;

	snprintf(filename, FNAME_LEN, "/dev/tty%d", *nr);
	err = stat(filename, &fstat);
	if (err < 0)
		return err;
	userinfo = getpwuid(fstat.st_uid);
	if (userinfo == NULL)
		return -1;
	*user = strdup(userinfo->pw_name);
	if (*user == NULL)
		return -1;

	return 0;
}

int acquire_new_vt(vt_t *vt) {
	int err;

	vt->nr = -1;
	vt->ios = NULL;
	vt->fd = -1;

	if (fd < 0)
		return -1;
	err = ioctl(fd, VT_OPENQRY, &vt->nr);
	if (err < 0)
		return err;
	err = ioctl(fd, VT_ACTIVATE, vt->nr);
	if (err < 0)
		return err;
	err = ioctl(fd, VT_WAITACTIVE, vt->nr);
	if (err < 0)
		return err;

	snprintf(filename, FNAME_LEN, "/dev/tty%d", vt->nr);
	vt->ios = fopen(filename, "r+");
	if (vt->ios == NULL)
		return -1;
	vt->fd = fileno(vt->ios);

	tcgetattr(vt->fd, &vt->term);
	vt->rlflag = vt->term.c_lflag;
	return 0;
}

int release_vt(vt_t *vt, int nr) {
	int err;

	if (fd < 0 || nr <= 0)
		return -1;
	err = ioctl(fd, VT_ACTIVATE, nr);
	if (err < 0)
		return err;
	err = ioctl(fd, VT_WAITACTIVE, nr);
	if (err < 0)
		return err;

	if (vt->ios != NULL) {
		fclose(vt->ios);
		vt->ios = NULL;
		vt->fd = -1;
	}

	if (vt->nr > 0) {
		err = ioctl(fd, VT_DISALLOCATE, vt->nr);
		if (err < 0)
			return err;
		vt->nr = -1;
	}
	
	return 0;
}

int lock_vt_switch() {
	int err;

	if (fd < 0)
		return -1;
	err = ioctl(fd, VT_LOCKSWITCH, 1);
	if (err < 0)
		return err;
	return 0;
}

int unlock_vt_switch() {
	int err;

	if (fd < 0)
		return -1;
	err = ioctl(fd, VT_UNLOCKSWITCH, 1);
	if (err < 0)
		return err;
	return 0;
}

int secure_vt(vt_t *vt) {
	if (vt->fd < 0)
		return -1;
	vt->term.c_lflag &= ~(ECHO | ISIG);
	tcsetattr(vt->fd, TCSANOW, &vt->term);
	return 0;
}

int tty_break_on(vt_t *vt) {
	if (vt->fd < 0)
		return -1;
	vt->term.c_lflag &= ~ICANON;
	tcsetattr(vt->fd, TCSANOW, &vt->term);
	return 0;
}

int tty_break_off(vt_t *vt) {
	if (vt->fd < 0)
		return -1;
	vt->term.c_lflag |= ICANON;
	tcsetattr(vt->fd, TCSANOW, &vt->term);
	return 0;
}

int reset_vt(vt_t *vt) {
	if (vt->fd < 0)
		return -1;
	vt->term.c_lflag = vt->rlflag;
	tcsetattr(vt->fd, TCSANOW, &vt->term);
	return 0;
}
