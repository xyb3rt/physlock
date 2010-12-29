#include "physlock.h"
#include "auth.h"
#include "options.h"
#include "vt.h"

#include <string.h>
#include <unistd.h>

#define PASSWD_LEN 1024

int oldvt;
vt_t vt;

int main(int argc, char **argv) {
	int only_root, auth = 0, len, chpid;
	char c, passwd[PASSWD_LEN];
	const char **uptr;
	userinfo_t *as, root, user;
	options_t options;

	oldvt = vt.nr = vt.fd = -1;
	vt.ios = NULL;
	root.name = "root";

	parse_options(argc, argv, &options);

	if (options.help) {
		print_usage();
		return 0;
	} else if (options.version) {
		print_version();
		return 0;
	} else if (options.only_lock) {
		vt_init();
		lock_vt_switch();
		vt_destroy();
		return 0;
	} else if (options.only_unlock) {
		vt_init();
		unlock_vt_switch();
		vt_destroy();
		return 0;
	}

	if (options.user != NULL) {
		uptr = NULL;
		user.name = options.user;
	} else {
		uptr = &user.name;
	}

	vt_init();
	get_current_vt(&oldvt, uptr);

	get_pwhash(&root);
	only_root = strcmp(user.name, "root") == 0;
	if (!only_root)
		get_pwhash(&user);

	acquire_new_vt(&vt);
	/*lock_vt_switch();*/
	secure_vt(&vt);

	if (options.bg) {
		chpid = fork();
		if (chpid < 0)
			FATAL("could not spawn background process");
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

		auth = authenticate(as, passwd);
		if (!auth) {
			fprintf(vt.ios, "\nAuthentication failed\n");
			sleep(1);
		}
	}

	clean_exit(0);

	return 0;
}

void clean_exit(int err) {
	static int in = 0;

	if (!in++) {
		if (vt.fd >= 0)
			reset_vt(&vt);
		unlock_vt_switch();
		release_vt(&vt, oldvt);
		vt_destroy();
	}

	exit(err);
}
