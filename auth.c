#define _XOPEN_SOURCE

#include <string.h>
#include <shadow.h>
#include <unistd.h>

#include "physlock.h"
#include "auth.h"

int authenticate(const char *username, const char *pw) {
	int ret;
	struct spwd *spw;
	char *cryptpw;

	setspent();

	spw = getspnam(username);
	if (spw == NULL)
		return -1;
	cryptpw = crypt(pw, spw->sp_pwdp);
	if (cryptpw == NULL)
		return -1;

	if (strcmp(cryptpw, spw->sp_pwdp) == 0)
		ret = 1;
	else
		ret = 0;
	
	endspent();

	return ret;
}
