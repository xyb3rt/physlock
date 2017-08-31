#include <stdlib.h>
#include <systemd/sd-login.h>

#include "auth.h"
#include "util.h"

void get_user(userinfo_t *uinfo, int vt, uid_t owner) {
	int found = 0, i, n;
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
			found = 1;
			break;
		}
	}

        if (sessions) {
		for (i = 0; i < n; i++)
			free(sessions[i]);
		free(sessions);
        }

	if (found)
		get_user_by_id(uinfo, sess_uid);
	else
		error(EXIT_FAILURE, 0, "Unable to detect user of tty%d", vt);
}

