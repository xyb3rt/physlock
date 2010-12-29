#ifndef AUTH_H
#define AUTH_H

typedef struct userinfo_s {
	const char *name;
	const char *pwhash;
} userinfo_t;

void get_pwhash(userinfo_t*);
int authenticate(const userinfo_t*, const char*);

#endif /* AUTH_H */
