/* physlock: signals.c
 * Copyright (c) 2011 Bert Muennich <muennich at informatik.hu-berlin.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *  
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *  
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "physlock.h"
#include "signals.h"

#include <string.h>
#include <errno.h>

void sa_handler_exit(int signum) {
	cleanup();
	exit(0);
}

int setup_signal(int signum, void (*handler)(int)) {
	struct sigaction sigact;

	sigact.sa_flags = 0;
	sigact.sa_handler = handler;
	sigemptyset(&sigact.sa_mask);
	
	if (sigaction(signum, &sigact, NULL) < 0) {
		WARN("could not set handler for signal %d: %s", signum, strerror(errno));
		return -1;
	} else {
		return 0;
	}
}

int signal_exit(int signum) {
	return setup_signal(signum, sa_handler_exit);
}

int signal_ignore(int signum) {
	return setup_signal(signum, SIG_IGN);
}
