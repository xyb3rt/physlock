#include "physlock.h"

#include <paths.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <unistd.h>
#include <errno.h>
#include <utmp.h>

#if HAVE_SYSTEMD
#include <systemd/sd-login.h>

int get_user_systemd(userinfo_t *uinfo, int vt) {
	int ret = -1, i, n;
	char **sessions = NULL;
	unsigned int sess_vt;
	uid_t sess_uid;

	n = sd_get_sessions(&sessions);

	for (i = 0; i < n; i++) {
		if (sd_session_get_vt(sessions[i], &sess_vt) < 0)
			continue;
		if (sess_vt == (unsigned)vt) {
			if (sd_session_get_uid(sessions[i], &sess_uid) < 0)
				continue;
			get_user_by_id(uinfo, sess_uid);
			ret = 0;
			break;
		}
	}

	for (i = 0; i < n; i++)
		free(sessions[i]);
	free(sessions);

	return ret;
}

#else

int get_user_systemd(userinfo_t *uinfo, int vt) {
	(void)uinfo;
	(void)vt;
	return -1;
}
#endif

int get_user_utmp(userinfo_t *uinfo, int vt) {
	int ret = -1;
	FILE *uf;
	struct utmp r;
	char tty[UT_LINESIZE+1], name[UT_NAMESIZE+1];

	name[0] = '\0';
	while ((uf = fopen(_PATH_UTMP, "r")) == NULL && errno == EINTR);

	if (uf != NULL) {
		snprintf(tty, sizeof(tty), "tty%d", vt);
		while (!feof(uf) && !ferror(uf)) {
			if (fread(&r, sizeof(r), 1, uf) != 1)
				continue;
			if (r.ut_type != USER_PROCESS || r.ut_user[0] == '\0')
				continue;
			if (strncmp(r.ut_line, tty, UT_LINESIZE) == 0) {
				strncpy(name, r.ut_user, UT_NAMESIZE);
				name[UT_NAMESIZE] = '\0';
				get_user_by_name(uinfo, name);
				ret = 0;
				break;
			}
		}
		fclose(uf);
	}
	return ret;
}

