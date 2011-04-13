/* physlock: main.c
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
#include "auth.h"
#include "options.h"
#include "vt.h"

#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#define PASSWD_LEN 1024

int oldvt;
vt_t vt;

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
		WARN("could not set handler for signal %d: %s", signum, strerror(errno));
		return -1;
	} else {
		return 0;
	}
}

int main(int argc, char **argv) {
	int only_root, auth = 0, len, chpid;
	char c, passwd[PASSWD_LEN];
	uid_t uid;
	userinfo_t *as, root, user;

	oldvt = vt.nr = vt.fd = -1;
	vt.ios = NULL;
	root.name = "root";

	parse_options(argc, argv);

	if (geteuid() != 0) {
		fprintf(stderr, "physlock: must be root!\n");
		return 1;
	}

	setup_signal(SIGTERM, sa_handler_exit);
	setup_signal(SIGQUIT, sa_handler_exit);
	setup_signal(SIGHUP, SIG_IGN);
	setup_signal(SIGINT, SIG_IGN);
	setup_signal(SIGUSR1, SIG_IGN);
	setup_signal(SIGUSR2, SIG_IGN);

	vt_init();

	if (options->only_lock) {
		lock_vt_switch();
		vt_destroy();
		return 0;
	} else if (options->only_unlock) {
		unlock_vt_switch();
		vt_destroy();
		return 0;
	}

	if (options->user) {
		user.name = options->user;
	} else {
		uid = getuid();
		get_uname(&user, uid);
	}

	get_current_vt(&oldvt);

	get_pwhash(&root);
	only_root = strcmp(user.name, "root") == 0;
	if (!only_root)
		get_pwhash(&user);

	acquire_new_vt(&vt);
	lock_vt_switch();
	secure_vt(&vt);

	if (options->detach) {
		chpid = fork();
		if (chpid < 0)
			DIE("could not spawn background process: %s", strerror(errno));
		else if (chpid > 0)
			return 0;
		else
			setsid();
	}

	while (!auth) {
		as = &root;
		if (!only_root) {
			tty_break_on(&vt);
			fprintf(vt.ios,
					"\nPress [R] to unlock as root or [U] to unlock as %s.\n",
					user.name);
			while (1) {
				c = fgetc(vt.ios);
				if (c == 'R' || c == 'r') {
					break;
				} else if (c == 'U' || c == 'u') {
					as = &user;
					break;
				}
				if (ferror(vt.ios))
					DIE("could not read from console: %s", strerror(errno));
			}
			tty_break_off(&vt);
		} else {
			fprintf(vt.ios, "\nPress [Enter] to unlock.\n");
			fgetc(vt.ios);
		}

		fprintf(vt.ios, "%s's password:", as->name);
		fgets(passwd, PASSWD_LEN, vt.ios);
		len = strlen(passwd);
		if (len > 0 && passwd[len-1] == '\n')
			passwd[len-1] = '\0';

		if (ferror(vt.ios))
			DIE("could not read from console: %s", strerror(errno));

		auth = authenticate(as, passwd);
		if (!auth) {
			fprintf(vt.ios, "\nAuthentication failed\n");
			sleep(AUTH_FAIL_TIMEOUT);
		}
	}

	cleanup();

	return 0;
}

void cleanup() {
	static int in = 0;

	if (!in++) {
		if (vt.fd >= 0)
			reset_vt(&vt);
		unlock_vt_switch();
		release_vt(&vt, oldvt);
		vt_destroy();
	}
}
