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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <unistd.h>
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

void get_user_by_id(userinfo_t *uinfo, uid_t uid) {
	struct passwd *pw;

	while (errno = 0, (pw = getpwuid(uid)) == NULL && errno == EINTR);
	if (pw == NULL)
		error(EXIT_FAILURE, 0, "No password file entry for uid %u found", uid);

	get_user_by_name(uinfo, pw->pw_name);
}

void get_user_by_name(userinfo_t *uinfo, const char *name) {
	uinfo->name = estrdup(name);
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
