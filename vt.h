#ifndef VT_H
#define VT_H

#include <termios.h>

typedef struct vt_s {
	int nr;
	FILE *ios;
	int fd;
	struct termios term;
	tcflag_t rlflag;
} vt_t;

void vt_init();
void vt_destroy();

void get_current_vt(int*, const char**);
void acquire_new_vt(vt_t*);
void release_vt(vt_t*, int);

void lock_vt_switch();
void unlock_vt_switch();

void secure_vt(vt_t*);
void tty_break_on(vt_t*);
void tty_break_off(vt_t*);
void reset_vt(vt_t*);

#endif /* VT_H */
