#include "auth.h"

void get_user(userinfo_t *uinfo, int vt, uid_t owner) {
	get_user_by_id(uinfo, owner);
}

