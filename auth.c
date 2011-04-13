/* physlock: auth.c
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

#include <string.h>
#include <pwd.h>
#include <shadow.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

void get_uname(userinfo_t *uinfo, uid_t uid) {
	struct passwd *pw;

	if (uinfo == NULL)
		return;

	pw = getpwuid(uid);
	if (pw == NULL)
		DIE("could not get user info for uid %u\n", uid);
	
	uinfo->name = strdup(pw->pw_name);
	if (uinfo->name == NULL)
		DIE("could not allocate memory");
}

void get_pwhash(userinfo_t *uinfo) {
	struct spwd *spw;

	if (uinfo == NULL || uinfo->name == NULL)
		return;

	setspent();

	spw = getspnam(uinfo->name);
	if (spw == NULL)
		DIE("could not get password hash for user %s", uinfo->name);

	uinfo->pwhash = strdup(spw->sp_pwdp);
	if (uinfo->pwhash == NULL)
		DIE("could not allocate memory");

	endspent();
}

int authenticate(const userinfo_t *uinfo, const char *pw) {
	char *cryptpw;

	if (uinfo == NULL || uinfo->pwhash == NULL || pw == NULL) {
		WARN("authenticate() called with invalid argument");
		return 0;
	}

	cryptpw = crypt(pw, uinfo->pwhash);
	if (cryptpw == NULL)
		DIE("could not hash password of user %s: %s", uinfo->name,
		      strerror(errno));

	return strcmp(cryptpw, uinfo->pwhash) == 0;
}
