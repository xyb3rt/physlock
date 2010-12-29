#include "physlock.h"
#include "auth.h"

#include <string.h>
#include <shadow.h>
#include <unistd.h>

void get_pwhash(userinfo_t *uinfo) {
	struct spwd *spw;

	if (uinfo == NULL || uinfo->name == NULL)
		return;

	setspent();

	spw = getspnam(uinfo->name);
	if (spw == NULL)
		FATAL("could not get password for user %s", uinfo->name);

	uinfo->pwhash = strdup(spw->sp_pwdp);
	if (uinfo->pwhash == NULL)
		FATAL("could not allocate memory");

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
		FATAL("could not hash password of user %s", uinfo->name);

	return strcmp(cryptpw, uinfo->pwhash) == 0;
}
