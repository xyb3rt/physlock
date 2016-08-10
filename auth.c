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

#include <paths.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <unistd.h>
#include <utmp.h>
#include <errno.h>
#include <security/pam_misc.h>

#include "auth.h"
#include "util.h"

static struct pam_conv conv = {
	misc_conv,
	NULL
};

static void get_pam(userinfo_t *uinfo) {
	if (pam_start("physlock", uinfo->name, &conv, &uinfo->pamh) != PAM_SUCCESS)
		error(EXIT_FAILURE, 0, "no pam for user %s", uinfo->name);
}

void get_user(userinfo_t *uinfo, int vt) {
	FILE *uf;
	struct utmp r;
	char tty[16], name[UT_NAMESIZE+1];

	while ((uf = fopen(_PATH_UTMP, "r")) == NULL && errno == EINTR);
	if (uf == NULL)
		error(EXIT_FAILURE, errno, "%s", _PATH_UTMP);

	uinfo->name = NULL;
	snprintf(tty, sizeof(tty), "tty%d", vt);

	while (!feof(uf) && !ferror(uf)) {
		if (fread(&r, sizeof(r), 1, uf) != 1)
			continue;
		if (r.ut_type != USER_PROCESS || r.ut_user[0] == '\0')
			continue;
		if (strcmp(r.ut_line, tty) == 0) {
			strncpy(name, r.ut_user, UT_NAMESIZE);
			name[UT_NAMESIZE] = '\0';
			uinfo->name = estrdup(name);
			break;
		}
	}
	fclose(uf);

	if (uinfo->name == NULL)
		error(EXIT_FAILURE, 0, "%s: No entry for %s found", _PATH_UTMP, tty);

	get_pam(uinfo);
}

void get_root(userinfo_t *uinfo) {
	struct passwd *pw;

	while (errno = 0, (pw = getpwuid(0)) == NULL && errno == EINTR);
	if (pw == NULL)
		error(EXIT_FAILURE, 0, "No password file entry for uid 0 found");

	uinfo->name = estrdup(pw->pw_name);

	get_pam(uinfo);
}

CLEANUP void free_user(userinfo_t *uinfo) {
	if (uinfo->pamh != NULL)
		pam_end(uinfo->pamh, uinfo->pam_status);
}

int authenticate(userinfo_t *uinfo) {
	uinfo->pam_status = pam_authenticate(uinfo->pamh, 0);

	if (uinfo->pam_status == PAM_SUCCESS)
		uinfo->pam_status = pam_acct_mgmt(uinfo->pamh, 0);

	return uinfo->pam_status == PAM_SUCCESS ? 0 : -1;
}
