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

static char buf[1024];
static int oldvt;
static vt_t vt;
static int oldsysrq;
static int oldprintk;
static pid_t chpid;

static userinfo_t root, user;

void cleanup() {
	if (options->detach && chpid > 0)
		/* No cleanup in parent after successful fork */
		return;
	free_user(&user);
	free_user(&root);
	if (oldsysrq > 0)
		write_int_to_file(SYSRQ_PATH, oldsysrq);
	if (oldprintk > 1)
		write_int_to_file(PRINTK_PATH, oldprintk);
	if (vt.fd >= 0)
		vt_reset(&vt);
	vt_lock_switch(0);
	vt_release(&vt, oldvt);
	vt_destroy();
}

void sa_handler_exit(int signum) {
	exit(0);
}

void setup_signal(int signum, void (*handler)(int)) {
	struct sigaction sigact;

	sigact.sa_flags = 0;
	sigact.sa_handler = handler;
	sigemptyset(&sigact.sa_mask);
	
	if (sigaction(signum, &sigact, NULL) < 0)
		error(0, errno, "signal %d", signum);
}

void prompt(FILE *stream, const char *fmt, ...) {
	va_list args;
	unsigned int c, i = 0;

	va_start(args, fmt);
	vfprintf(stream, fmt, args);
	va_end(args);

	for (;;) {
		while ((c = fgetc(stream)) == EOF && errno == EINTR);
		if (c == EOF || c == '\n')
			break;
		if (c != '\0' && i + 1 < sizeof(buf))
			buf[i++] = (char) c;
	}
	if (ferror(stream))
		error(EXIT_FAILURE, 0, "Error reading from console");
	buf[i] = '\0';
}

int main(int argc, char **argv) {
	int try = 0, unauth = 1, user_only = 1;
	userinfo_t *u = &user;

	oldvt = oldsysrq = oldprintk = vt.nr = vt.fd = -1;
	vt.ios = NULL;

	error_init(2);
	parse_options(argc, argv);

	if (geteuid() != 0)
		error(EXIT_FAILURE, 0, "Must be root!");

	setup_signal(SIGTERM, sa_handler_exit);
	setup_signal(SIGQUIT, sa_handler_exit);
	setup_signal(SIGHUP, SIG_IGN);
	setup_signal(SIGINT, SIG_IGN);
	setup_signal(SIGUSR1, SIG_IGN);
	setup_signal(SIGUSR2, SIG_IGN);

	vt_init();
	vt_get_current(&oldvt);

	if (options->lock_switch != -1) {
		if (vt_lock_switch(options->lock_switch) == -1)
			exit(EXIT_FAILURE);
		vt_destroy();
		return 0;
	}

	get_user(&user, oldvt);
	get_root(&root);
	if (strcmp(user.name, root.name) != 0)
		user_only = 0;

	atexit(cleanup);

	if (options->disable_sysrq) {
		oldsysrq = read_int_from_file(SYSRQ_PATH, '\n');
		if (oldsysrq > 0)
			if (write_int_to_file(SYSRQ_PATH, 0) == -1)
				exit(EXIT_FAILURE);
	}

	if (options->mute_kernel_messages) {
		oldprintk = read_int_from_file(PRINTK_PATH, '\t');
		if (oldprintk > 1)
			if (write_int_to_file(PRINTK_PATH, 1) == -1)
				exit(EXIT_FAILURE);
	}

	vt_acquire(&vt);
	vt_lock_switch(1);

	if (options->detach) {
		chpid = fork();
		if (chpid < 0) {
			error(EXIT_FAILURE, errno, "fork");
		} else if (chpid > 0) {
			return 0;
		} else {
			setsid();
			sleep(1); /* w/o this, accessing the vt might fail */
			vt_reopen(&vt);
		}
	}
	vt_secure(&vt);

	dup2(vt.fd, 0);
	dup2(vt.fd, 1);
	dup2(vt.fd, 2);

	openlog(progname, LOG_PID, LOG_AUTH);

	while (unauth) {
		if (u == &root) {
			fprintf(vt.ios, "%s's ", root.name);
			fflush(vt.ios);
		}
		unauth = authenticate(u);
		if (unauth) {
			if (!user_only && (u == &root || ++try == 3)) {
				u = u == &root ? &user : &root;
				try = 0;
			}
			fprintf(vt.ios, "Authentication failed\n\n");
			syslog(LOG_WARNING, "Authentication failure");
		}
	}

	return 0;
}
