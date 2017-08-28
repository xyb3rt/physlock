/* physlock: auth.h
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

#ifndef AUTH_H
#define AUTH_H

#include <security/pam_appl.h>

#define CLEANUP

typedef struct userinfo_s {
	const char *name;
	int pam_status;
	pam_handle_t *pamh;
} userinfo_t;

void get_user(userinfo_t*, int, uid_t);
void get_user_systemd(userinfo_t*, int, uid_t);
void get_root(userinfo_t*);
CLEANUP void free_user(userinfo_t*);
int authenticate(userinfo_t*);

#endif /* AUTH_H */
