/* physlock: main.c
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <syslog.h>

#include "auth.h"
#include "config.h"
#include "options.h"
#include "util.h"
#include "vt.h"

enum { BUFLEN = 1024 };

const char *progname;
static char buf[BUFLEN];
static int oldvt;
static vt_t vt;
static int oldsysrq;
static int oldprintk;

void cleanup() {
	static int in = 0;

	if (!in++) {
		if (oldsysrq > 0)
			write_int_to_file(SYSRQ_PATH, oldsysrq);
		if (oldprintk > 1)
			write_int_to_file(PRINTK_PATH, oldprintk);
		if (vt.fd >= 0)
			reset_vt(&vt);
		unlock_vt_switch();
		release_vt(&vt, oldvt);
		vt_destroy();
		closelog();
		memset(buf, 0, sizeof(buf));
	}
}

void sa_handler_exit(int signum) {
	cleanup();
	exit(0);
}

int setup_signal(int signum, void (*handler)(int)) {
	struct sigaction sigact;

	sigact.sa_flags = 0;
	sigact.sa_handler = handler;
	sigemptyset(&sigact.sa_mask);
	
	if (sigaction(signum, &sigact, NULL) < 0) {
		warn("could not set handler for signal %d: %s", signum, strerror(errno));
		return -1;
	} else {
		return 0;
	}
}

void prompt(FILE *stream, const char *fmt, ...) {
	va_list args;
	unsigned int c, i = 0;

	va_start(args, fmt);
	vfprintf(stream, fmt, args);
	va_end(args);

	while ((c = fgetc(stream)) != EOF && c != '\n') {
		if (c != '\0' && i + 1 < BUFLEN)
			buf[i++] = (char) c;
	}
	if (ferror(stream))
		die("could not read from console: %s", strerror(errno));
	buf[i] = '\0';
}

int main(int argc, char **argv) {
	int try = 0, unauth = 1, user_only = 1;
	userinfo_t root, user, *u = &user;

	progname = s_basename(argv[0]);
	oldvt = oldsysrq = oldprintk = vt.nr = vt.fd = -1;
	vt.ios = NULL;

	parse_options(argc, argv);

	if (geteuid() != 0) {
		fprintf(stderr, "%s: must be root!\n", progname);
		return 1;
	}

	setup_signal(SIGTERM, sa_handler_exit);
	setup_signal(SIGQUIT, sa_handler_exit);
	setup_signal(SIGHUP, SIG_IGN);
	setup_signal(SIGINT, SIG_IGN);
	setup_signal(SIGUSR1, SIG_IGN);
	setup_signal(SIGUSR2, SIG_IGN);

	close(0);
	close(1);

	openlog(progname, LOG_PID, LOG_AUTH);

	vt_init();
	get_current_vt(&oldvt);

	if (options->only_lock) {
		lock_vt_switch();
		vt_destroy();
		return 0;
	} else if (options->only_unlock) {
		unlock_vt_switch();
		vt_destroy();
		return 0;
	}

	get_user(&user, oldvt);
	if (authenticate(&user, "") == -1)
		die("could not hash password for user %s", user.name);
	get_root(&root);
	if (strcmp(user.name, root.name) != 0 && authenticate(&root, "") != -1)
		user_only = 0;

	if (options->disable_sysrq) {
		oldsysrq = read_int_from_file(SYSRQ_PATH, '\n');
		if (oldsysrq > 0)
			write_int_to_file(SYSRQ_PATH, 0);
	}

	if (options->mute_kernel_messages) {
		oldprintk = read_int_from_file(PRINTK_PATH, '\t');
		if (oldprintk > 1)
			write_int_to_file(PRINTK_PATH, 1);
	}

	acquire_new_vt(&vt);
	lock_vt_switch();

	if (options->detach) {
		int chpid = fork();
		if (chpid < 0) {
			die("could not spawn background process: %s", strerror(errno));
		} else if (chpid > 0) {
			return 0;
		} else {
			setsid();
			sleep(1); /* w/o this, accessing the vt might fail */
			reopen_vt(&vt);
		}
	}
	secure_vt(&vt);

	while (unauth || unauth == 1000) {
		flush_vt(&vt);
		prompt(vt.ios, "%s's password: ", u->name);
		unauth = authenticate(u, buf);
		memset(buf, 0, sizeof(buf));
		if (unauth) {
			fprintf(vt.ios, "\nAuthentication failed\n\n");
			syslog(LOG_WARNING, "Authentication failure");
			sleep(AUTH_FAIL_TIMEOUT);
		}
	}
	cleanup();

	return 0;
}
