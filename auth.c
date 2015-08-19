/* physlock: auth.c
 * Copyright (c) 2013,2015 Bert Muennich <be.muennich at gmail.com>
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
#define _XOPEN_SOURCE   500 /* for crypt() and strdup() */

#include <paths.h>
#include <stdio.h>
#include <string.h>
#include <shadow.h>
#include <pwd.h>
#include <unistd.h>
#include <utmp.h>
#include <errno.h>

#include "auth.h"
#include "util.h"

void get_user(userinfo_t *uinfo, int vt) {
	FILE *uf;
	struct utmp r;
	struct spwd *spw;
	char tty[16];

	if (uinfo == NULL)
		return;

	uf = fopen(_PATH_UTMP, "r");
	if (uf == NULL)
		die("could not open: %s: %s", _PATH_UTMP, strerror(errno));

	uinfo->name = NULL;
	snprintf(tty, sizeof(tty), "tty%d", vt);

	while (!feof(uf) && !ferror(uf)) {
		if (fread(&r, sizeof(r), 1, uf) != 1)
			continue;
		if (r.ut_type != USER_PROCESS || r.ut_user[0] == '\0')
			continue;
		if (strcmp(r.ut_line, tty) == 0) {
			uinfo->name = strdup(r.ut_user);
			if (uinfo->name == NULL)
				die("could not allocate memory");
			break;
		}
	}
	fclose(uf);

	if (uinfo->name == NULL)
		die("could not identify active user");

	setspent();

	spw = getspnam(uinfo->name);
	if (spw == NULL)
		die("could not get password hash of user %s", uinfo->name);

	uinfo->pwhash = strdup(spw->sp_pwdp);
	if (uinfo->pwhash == NULL)
		die("could not allocate memory");

	endspent();
}

int authenticate(const userinfo_t *uinfo, const char *pw) {
	char *cryptpw;

	if (uinfo == NULL || uinfo->pwhash == NULL || pw == NULL)
		return 0;

	cryptpw = crypt(pw, uinfo->pwhash);
	if (cryptpw == NULL)
		die("could not hash password for user %s", uinfo->name);

	return strcmp(cryptpw, uinfo->pwhash) == 0;
}
