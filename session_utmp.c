#include "physlock.h"

#include <paths.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <unistd.h>
#include <errno.h>
#include <utmp.h>

void get_user(userinfo_t *uinfo, int vt, uid_t owner) {
	int found = 0;
	FILE *uf;
	struct utmp r;
	char tty[16], name[UT_NAMESIZE+1];

	while ((uf = fopen(_PATH_UTMP, "r")) == NULL && errno == EINTR);

	if (uf != NULL) {
		snprintf(tty, sizeof(tty), "tty%d", vt);
		while (!feof(uf) && !ferror(uf)) {
			if (fread(&r, sizeof(r), 1, uf) != 1)
				continue;
			if (r.ut_type != USER_PROCESS || r.ut_user[0] == '\0')
				continue;
			strncpy(name, r.ut_user, UT_NAMESIZE);
			name[UT_NAMESIZE] = '\0';
			if (strcmp(name, tty) == 0) {
				found = 1;
				break;
			}
		}
		fclose(uf);
	}
	if (found)
		get_user_by_name(uinfo, name);
	else
		error(EXIT_FAILURE, 0, "No utmp entry for tty%d found", vt);
}

