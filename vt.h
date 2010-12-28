#ifndef VT_H
#define VT_H

#include <stdio.h>
#include <termios.h>

typedef struct vt_s {
	int nr;
	FILE *ios;
	int fd;
	struct termios term;
	tcflag_t rlflag;
} vt_t;

int vt_init();
void vt_destroy();

int get_current_vt(int*, char**);
int acquire_new_vt(vt_t*);
int release_vt(vt_t*, int);

int lock_vt_switch();
int unlock_vt_switch();

int secure_vt(vt_t*);
int tty_break_on(vt_t*);
int tty_break_off(vt_t*);
int reset_vt(vt_t*);

#endif /* VT_H */
