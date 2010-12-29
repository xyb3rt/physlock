#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "physlock.h"
#include "auth.h"
#include "options.h"
#include "vt.h"

#define PASSWD_LEN 1024

int oldvt;
vt_t vt;

int main(int argc, char **argv) {
	int auth = 0, len, chpid;
	char c, passwd[PASSWD_LEN];
	const char *as, *username, **uptr;
	options_t options;

	oldvt = vt.nr = vt.fd = -1;
	vt.ios = NULL;

	parse_options(argc, argv, &options);

	if (options.help) {
		print_usage();
		return 0;
	} else if (options.version) {
		print_version();
		return 0;
	}

	if (options.user != NULL) {
		uptr = NULL;
		username = options.user;
	} else {
		uptr = &username;
	}

	if (vt_init() < 0)
		FATAL("could not open console device");
	
	if (options.o_lock) {
		if (lock_vt_switch() < 0)
			FATAL("could not lock console switching");
		vt_destroy();
		return 0;
	} else if (options.o_unlock) {
		if (unlock_vt_switch() < 0)
			WARN("could not enable console switching");
		vt_destroy();
		return 0;
	}

	if (get_current_vt(&oldvt, uptr) < 0)
		FATAL("could not get console state");

	if (acquire_new_vt(&vt) < 0)
		FATAL("could not aquire new console");

	if (lock_vt_switch() < 0)
		FATAL("could not lock console switching");

	if (secure_vt(&vt) < 0)
		FATAL("could not secure console");

	if (!options.fg) {
		chpid = fork();
		if (chpid < 0)
			FATAL("could not spawn background process");
		else if (chpid > 0)
			return 0;
	}

	while (!auth) {
		as = username;
		if (strcmp(username, "root") != 0) {
			tty_break_on(&vt);
			fprintf(vt.ios,
					"\nPress [R] to unlock as root or [U] to unlock as %s.\n",
					username);
			while (1) {
				c = fgetc(vt.ios);
				if (c == 'R' || c == 'r') {
					as = "root";
					break;
				} else if (c == 'U' || c == 'u') {
					break;
				}
			}
			tty_break_off(&vt);
		} else {
			fprintf(vt.ios, "\nPress [Enter] to unlock.\n");
			fgetc(vt.ios);
		}

		fprintf(vt.ios, "%s's password:", as);
		fgets(passwd, PASSWD_LEN, vt.ios);
		len = strlen(passwd);
		if (len > 0 && passwd[len-1] == '\n')
			passwd[len-1] = '\0';

		auth = authenticate(as, passwd);
		if (auth < 0) {
			FATAL("could not read user db");
		} else if (!auth) {
			fprintf(vt.ios, "\nAuthentication failed\n");
			sleep(1);
		}
	}

	cleanup(1);

	return 0;
}

void cleanup(int warn) {
	if (reset_vt(&vt) < 0 && warn)
		WARN("could not reset console mode");

	if (unlock_vt_switch() < 0 && warn)
		WARN("could not enable console switching");

	if (release_vt(&vt, oldvt) < 0 && warn)
		WARN("could not release console");

	vt_destroy();
}
